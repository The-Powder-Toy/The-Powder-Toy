#pragma once

#include "Snapshot.h"

#include <memory>
#include <cstdint>

struct SnapshotDelta
{
	template<class Item>
	struct Diff
	{
		Item oldItem, newItem;
	};

	template<class Item>
	struct Hunk
	{
		int offset;
		std::vector<Diff<Item>> diffs;
	};

	template<class Item>
	struct SingleDiff
	{
		bool valid = false;
		Diff<Item> diff;
	};

	template<class Item>
	struct HalfHunk
	{
		int offset;
		std::vector<Item> items;
	};

	template<class Item>
	using HunkVector = std::vector<Hunk<Item>>;

	template<class Item>
	struct HalfHunkVectorPair
	{
		std::vector<HalfHunk<Item>> oldHunks, newHunks;
	};

	HunkVector<float> AirPressure;
	HunkVector<float> AirVelocityX;
	HunkVector<float> AirVelocityY;
	HunkVector<float> AmbientHeat;

	HunkVector<uint32_t> commonParticles;
	std::vector<Particle> extraPartsOld, extraPartsNew;

	HunkVector<float> GravVelocityX;
	HunkVector<float> GravVelocityY;
	HunkVector<float> GravValue;
	HunkVector<float> GravMap;

	HunkVector<unsigned char> BlockMap;
	HunkVector<unsigned char> ElecMap;

	HunkVector<float> FanVelocityX;
	HunkVector<float> FanVelocityY;


	HunkVector<uint32_t> PortalParticles;
	HunkVector<int> WirelessData;
	HunkVector<uint32_t> stickmen;
	SingleDiff<std::vector<sign>> signs;

	SingleDiff<Json::Value> Authors;

	static std::unique_ptr<SnapshotDelta> FromSnapshots(const Snapshot &oldSnap, const Snapshot &newSnap);
	std::unique_ptr<Snapshot> Forward(const Snapshot &oldSnap);
	std::unique_ptr<Snapshot> Restore(const Snapshot &newSnap);
};
