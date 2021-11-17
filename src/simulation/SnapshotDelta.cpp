#include "SnapshotDelta.h"

#include "common/tpt-minmax.h"

#include <utility>

// * A SnapshotDelta is a bidirectional difference type between Snapshots, defined such
//   that SnapshotDelta d = SnapshotDelta::FromSnapshots(A, B) yields a SnapshotDelta which can be
//   used to construct a Snapshot identical to A via d.Restore(B) and a Snapshot identical
//   to B via d.Forward(A). Thus, d = B - A, A = B - d and B = A + d.
// * Fields in Snapshot can be classified into two groups:
//   * Fields of static size, whose sizes are identical to the size of the corresponding field
//     in all other Snapshots. Example of these fields include AmbientHeat (whose size depends
//     on XRES, YRES and CELL, all compile-time constants) and WirelessData (whose size depends
//     on CHANNELS, another compile-time constant). Note that these fields would be of "static
//     size" even if their sizes weren't derived from compile-time constants, as they'd still
//     be the same size throughout the life of a Simulation, and thus any Snapshot created from it.
//   * Fields of dynamic size, whose sizes may be different between Snapshots. These are, fortunately,
//     the minority: Particles, signs and Authors.
// * Each field in Snapshot has a mirror set of fields in SnapshotDelta. Fields of static size
//   have mirror fields whose type is HunkVector, templated by the item type of the
//   corresponding field; these fields are handled in a uniform manner. Fields of dynamic size are
//   handled in a non-uniform, case-by-case manner. 
// * A HunkVector is generated from two streams of identical size and is a collection
//   of Hunks, a Hunk is an offset combined with a collection of Diffs, and a Diff is a pair of values,
//   one originating from one stream and the other from the other. Thus, Hunks represent contiguous
//   sequences of differences between the two streams, and a HunkVector is a compact way to represent
//   all differences between the two streams it's generated from. In this case, these streams are
//   the data in corresponding fields of static size in two Snapshots, and the HunkVector is the
//   respective field in the SnapshotDelta that is the difference between the two Snapshots.
//   * FillHunkVectorPtr is the d = B - A operation, which takes two Snapshot fields of static size and
//     the corresponding SnapshotDelta field, and fills the latter with the HunkVector generated
//     from the former streams.
//   * ApplyHunkVector<true> is the A = B - d operation, which takes a field of a SnapshotDelta and
//     the corresponding field of a "newer" Snapshot, and fills the latter with the "old" values.
//   * ApplyHunkVector<false> is the B = A + d operation, which takes a field of a SnapshotDelta and
//     the corresponding field of an "older" Snapshot, and fills the latter with the "new" values.
//   * This difference type is intended for fields of static size. This covers all fields in Snapshot
//     except for Particles, signs, and Authors.
// * A SingleDiff is, unsurprisingly enough, a single Diff, with an accompanying bool that signifies
//   whether the Diff does in fact hold the "old" value of a field in the "old" Snapshot and the "new"
//   value of the same field in the "new" Snapshot. If this bool is false, the data in the fields
//   of both Snapshots are equivalent and the SingleDiff should be ignored. If it's true, the
//   SingleDiff represents the difference between these fields.
//   * FillSingleDiff is the d = B - A operation, while ApplySingleDiff<false> and ApplySingleDiff<true>
//     are the A = B - d and B = A + d operations. These are self-explanatory.
//   * This difference type is intended for fields of dynamic size whose data doesn't change often and
//     doesn't consume too much memory. This covers the Snapshot fields signs and Authors.
// * This leaves Snapshot::Particles. This field mirrors Simulation::parts, which is actually also
//   a field of static size, but since most of the time most of this array is empty, it doesn't make
//   sense to store all of it in a Snapshot (unlike Air::hv, which can be fairly chaotic (i.e. may have
//   a lot of interesting data in all of its cells) when ambient heat simulation is enabled, or
//   Simulation::wireless, which is not big enough to need compression). This makes Snapshots smaller,
//   but the life of a SnapshotDelta developer harder. The following, relatively simple approach is
//   taken, as a sort of compromise between simplicity and memory usage:
//   * The common part of the Particles arrays in the old and the new Snapshots is identified: this is
//     the overlapping part, i.e. the first size cells of both arrays, where
//     size = min(old.Particles.size(), new.Particles.size()), and a HunkVector is generated from it,
//     as though it was a field of static size. For our purposes, it is indeed Static Enough:tm:, for
//     it only needs to be the same size as the common part of the Particles arrays of the two Snapshots.
//   * The rest of both Particles arrays is copied to the extra fields extraPartsOld and extraPartsNew.
// * One more trick is at work here: Particle structs are actually compared property-by-property rather
//   than as a whole. This ends up being beneficial to memory usage, as many properties (e.g. type
//   and ctype) don't often change over time, while others (e.g. x and y) do. Currently, all Particle
//   properties are 4-byte integral values, which makes it feasible to just reinterpret_cast Particle
//   structs as arrays of uint32_t values and generate HunkVectors from the resulting streams instead.
//   This assumption is enforced by the following static_asserts. The same trick is used for playerst
//   structs, even though Snapshot::stickmen is not big enough for us to benefit from this. The
//   alternative would have been to implement operator ==(const playerst &, const playerst &), which
//   would have been tedious.

constexpr size_t ParticleUint32Count = sizeof(Particle) / sizeof(uint32_t);
static_assert(sizeof(Particle) % sizeof(uint32_t) == 0, "fix me");

constexpr size_t playerstUint32Count = sizeof(playerst) / sizeof(uint32_t);
static_assert(sizeof(playerst) % sizeof(uint32_t) == 0, "fix me");

// * Needed by FillHunkVector for handling Snapshot::stickmen.
bool operator ==(const playerst &lhs, const playerst &rhs)
{
	auto match = true;
	for (auto i = 0U; i < 16U; ++i)
	{
		match = match && lhs.legs[i] == rhs.legs[i];
	}
	for (auto i = 0U; i < 8U; ++i)
	{
		match = match && lhs.accs[i] == rhs.accs[i];
	}
	return match                              &&
	       lhs.comm        == rhs.comm        &&
	       lhs.pcomm       == rhs.pcomm       &&
	       lhs.elem        == rhs.elem        &&
	       lhs.spwn        == rhs.spwn        &&
	       lhs.frames      == rhs.frames      &&
	       lhs.rocketBoots == rhs.rocketBoots &&
	       lhs.fan         == rhs.fan         &&
	       lhs.spawnID     == rhs.spawnID;
}

// * Needed by FillSingleDiff for handling Snapshot::signs.
bool operator ==(const std::vector<sign> &lhs, const std::vector<sign> &rhs)
{
	if (lhs.size() != rhs.size())
	{
		return false;
	}
	for (auto i = 0U; i < lhs.size(); ++i)
	{
		if (!(lhs[i].x    == rhs[i].x    &&
		      lhs[i].y    == rhs[i].y    &&
		      lhs[i].ju   == rhs[i].ju   &&
		      lhs[i].text == rhs[i].text))
		{
			return false;
		}
	}
	return true;
}

template<class Item>
void FillHunkVectorPtr(const Item *oldItems, const Item *newItems, SnapshotDelta::HunkVector<Item> &out, size_t size)
{
	auto i = 0U;
	bool different = false;
	auto offset = 0U;
	auto markDifferent = [oldItems, newItems, &out, &i, &different, &offset](bool mark) {
		if (mark && !different)
		{
			different = true;
			offset = i;
		}
		else if (!mark && different)
		{
			different = false;
			auto size = i - offset;
			out.emplace_back();
			auto &hunk = out.back();
			hunk.offset = offset;
			auto &diffs = hunk.diffs;
			diffs.resize(size);
			for (auto j = 0U; j < size; ++j)
			{
				diffs[j].oldItem = oldItems[offset + j];
				diffs[j].newItem = newItems[offset + j];
			}
		}
	};
	while (i < size)
	{
		markDifferent(!(oldItems[i] == newItems[i]));
		i += 1U;
	}
	markDifferent(false);
}

template<class Item>
void FillHunkVector(const std::vector<Item> &oldItems, const std::vector<Item> &newItems, SnapshotDelta::HunkVector<Item> &out)
{
	FillHunkVectorPtr<Item>(&oldItems[0], &newItems[0], out, std::min(oldItems.size(), newItems.size()));
}

template<class Item>
void FillSingleDiff(const Item &oldItem, const Item &newItem, SnapshotDelta::SingleDiff<Item> &out)
{
	if (oldItem != newItem)
	{
		out.valid = true;
		out.diff.oldItem = oldItem;
		out.diff.newItem = newItem;
	}
}

template<bool UseOld, class Item>
void ApplyHunkVectorPtr(const SnapshotDelta::HunkVector<Item> &in, Item *items)
{
	for (auto &hunk : in)
	{
		auto offset = hunk.offset;
		auto &diffs = hunk.diffs;
		for (auto j = 0U; j < diffs.size(); ++j)
		{
			items[offset + j] = UseOld ? diffs[j].oldItem : diffs[j].newItem;
		}
	}
}

template<bool UseOld, class Item>
void ApplyHunkVector(const SnapshotDelta::HunkVector<Item> &in, std::vector<Item> &items)
{
	ApplyHunkVectorPtr<UseOld, Item>(in, &items[0]);
}

template<bool UseOld, class Item>
void ApplySingleDiff(const SnapshotDelta::SingleDiff<Item> &in, Item &item)
{
	if (in.valid)
	{
		item = UseOld ? in.diff.oldItem : in.diff.newItem;
	}
}

std::unique_ptr<SnapshotDelta> SnapshotDelta::FromSnapshots(const Snapshot &oldSnap, const Snapshot &newSnap)
{
	auto ptr = std::make_unique<SnapshotDelta>();
	auto &delta = *ptr;
	FillHunkVector(oldSnap.AirPressure    , newSnap.AirPressure    , delta.AirPressure    );
	FillHunkVector(oldSnap.AirVelocityX   , newSnap.AirVelocityX   , delta.AirVelocityX   );
	FillHunkVector(oldSnap.AirVelocityY   , newSnap.AirVelocityY   , delta.AirVelocityY   );
	FillHunkVector(oldSnap.AmbientHeat    , newSnap.AmbientHeat    , delta.AmbientHeat    );
	FillHunkVector(oldSnap.GravVelocityX  , newSnap.GravVelocityX  , delta.GravVelocityX  );
	FillHunkVector(oldSnap.GravVelocityY  , newSnap.GravVelocityY  , delta.GravVelocityY  );
	FillHunkVector(oldSnap.GravValue      , newSnap.GravValue      , delta.GravValue      );
	FillHunkVector(oldSnap.GravMap        , newSnap.GravMap        , delta.GravMap        );
	FillHunkVector(oldSnap.BlockMap       , newSnap.BlockMap       , delta.BlockMap       );
	FillHunkVector(oldSnap.ElecMap        , newSnap.ElecMap        , delta.ElecMap        );
	FillHunkVector(oldSnap.FanVelocityX   , newSnap.FanVelocityX   , delta.FanVelocityX   );
	FillHunkVector(oldSnap.FanVelocityY   , newSnap.FanVelocityY   , delta.FanVelocityY   );
	FillHunkVector(oldSnap.WirelessData   , newSnap.WirelessData   , delta.WirelessData   );
	FillSingleDiff(oldSnap.signs          , newSnap.signs          , delta.signs          );
	FillSingleDiff(oldSnap.Authors        , newSnap.Authors        , delta.Authors        );
	FillHunkVectorPtr(reinterpret_cast<const uint32_t *>(&oldSnap.PortalParticles[0]), reinterpret_cast<const uint32_t *>(&newSnap.PortalParticles[0]), delta.PortalParticles, newSnap.PortalParticles.size() * ParticleUint32Count);
	FillHunkVectorPtr(reinterpret_cast<const uint32_t *>(&oldSnap.stickmen[0])       , reinterpret_cast<const uint32_t *>(&newSnap.stickmen[0]       ), delta.stickmen       , newSnap.stickmen       .size() * playerstUint32Count);

	// * Slightly more interesting; this will only diff the common parts, the rest is copied separately.
	auto commonSize = std::min(oldSnap.Particles.size(), newSnap.Particles.size());
	FillHunkVectorPtr(reinterpret_cast<const uint32_t *>(&oldSnap.Particles[0]), reinterpret_cast<const uint32_t *>(&newSnap.Particles[0]), delta.commonParticles, commonSize * ParticleUint32Count);
	delta.extraPartsOld.resize(oldSnap.Particles.size() - commonSize);
	std::copy(oldSnap.Particles.begin() + commonSize, oldSnap.Particles.end(), delta.extraPartsOld.begin());
	delta.extraPartsNew.resize(newSnap.Particles.size() - commonSize);
	std::copy(newSnap.Particles.begin() + commonSize, newSnap.Particles.end(), delta.extraPartsNew.begin());

	return ptr;
}

std::unique_ptr<Snapshot> SnapshotDelta::Forward(const Snapshot &oldSnap)
{
	auto ptr = std::make_unique<Snapshot>(oldSnap);
	auto &newSnap = *ptr;
	ApplyHunkVector<false>(AirPressure    , newSnap.AirPressure    );
	ApplyHunkVector<false>(AirVelocityX   , newSnap.AirVelocityX   );
	ApplyHunkVector<false>(AirVelocityY   , newSnap.AirVelocityY   );
	ApplyHunkVector<false>(AmbientHeat    , newSnap.AmbientHeat    );
	ApplyHunkVector<false>(GravVelocityX  , newSnap.GravVelocityX  );
	ApplyHunkVector<false>(GravVelocityY  , newSnap.GravVelocityY  );
	ApplyHunkVector<false>(GravValue      , newSnap.GravValue      );
	ApplyHunkVector<false>(GravMap        , newSnap.GravMap        );
	ApplyHunkVector<false>(BlockMap       , newSnap.BlockMap       );
	ApplyHunkVector<false>(ElecMap        , newSnap.ElecMap        );
	ApplyHunkVector<false>(FanVelocityX   , newSnap.FanVelocityX   );
	ApplyHunkVector<false>(FanVelocityY   , newSnap.FanVelocityY   );
	ApplyHunkVector<false>(WirelessData   , newSnap.WirelessData   );
	ApplySingleDiff<false>(signs          , newSnap.signs          );
	ApplySingleDiff<false>(Authors        , newSnap.Authors        );
	ApplyHunkVectorPtr<false>(PortalParticles, reinterpret_cast<uint32_t *>(&newSnap.PortalParticles[0]));
	ApplyHunkVectorPtr<false>(stickmen       , reinterpret_cast<uint32_t *>(&newSnap.stickmen[0]       ));

	// * Slightly more interesting; apply the common hunk vector, copy the extra portion separaterly.
	ApplyHunkVectorPtr<false>(commonParticles, reinterpret_cast<uint32_t *>(&newSnap.Particles[0]));
	auto commonSize = oldSnap.Particles.size() - extraPartsOld.size();
	newSnap.Particles.resize(commonSize + extraPartsNew.size());
	std::copy(extraPartsNew.begin(), extraPartsNew.end(), newSnap.Particles.begin() + commonSize);

	return ptr;
}

std::unique_ptr<Snapshot> SnapshotDelta::Restore(const Snapshot &newSnap)
{
	auto ptr = std::make_unique<Snapshot>(newSnap);
	auto &oldSnap = *ptr;
	ApplyHunkVector<true>(AirPressure    , oldSnap.AirPressure    );
	ApplyHunkVector<true>(AirVelocityX   , oldSnap.AirVelocityX   );
	ApplyHunkVector<true>(AirVelocityY   , oldSnap.AirVelocityY   );
	ApplyHunkVector<true>(AmbientHeat    , oldSnap.AmbientHeat    );
	ApplyHunkVector<true>(GravVelocityX  , oldSnap.GravVelocityX  );
	ApplyHunkVector<true>(GravVelocityY  , oldSnap.GravVelocityY  );
	ApplyHunkVector<true>(GravValue      , oldSnap.GravValue      );
	ApplyHunkVector<true>(GravMap        , oldSnap.GravMap        );
	ApplyHunkVector<true>(BlockMap       , oldSnap.BlockMap       );
	ApplyHunkVector<true>(ElecMap        , oldSnap.ElecMap        );
	ApplyHunkVector<true>(FanVelocityX   , oldSnap.FanVelocityX   );
	ApplyHunkVector<true>(FanVelocityY   , oldSnap.FanVelocityY   );
	ApplyHunkVector<true>(WirelessData   , oldSnap.WirelessData   );
	ApplySingleDiff<true>(signs          , oldSnap.signs          );
	ApplySingleDiff<true>(Authors        , oldSnap.Authors        );
	ApplyHunkVectorPtr<true>(PortalParticles, reinterpret_cast<uint32_t *>(&oldSnap.PortalParticles[0]));
	ApplyHunkVectorPtr<true>(stickmen       , reinterpret_cast<uint32_t *>(&oldSnap.stickmen[0]       ));

	// * Slightly more interesting; apply the common hunk vector, copy the extra portion separaterly.
	ApplyHunkVectorPtr<true>(commonParticles, reinterpret_cast<uint32_t *>(&oldSnap.Particles[0]));
	auto commonSize = newSnap.Particles.size() - extraPartsNew.size();
	oldSnap.Particles.resize(commonSize + extraPartsOld.size());
	std::copy(extraPartsOld.begin(), extraPartsOld.end(), oldSnap.Particles.begin() + commonSize);

	return ptr;
}
