#include "SaveRenderer.h"

#include "client/GameSave.h"

#include "graphics/VideoBuffer.h"
#include "graphics/Renderer.h"

#include "Simulation.h"
#include "SimulationData.h"

SaveRenderer::SaveRenderer()
{
	sim = std::make_unique<Simulation>();
	ren = std::make_unique<Renderer>();
	ren->sim = sim.get();
}

SaveRenderer::~SaveRenderer() = default;

std::unique_ptr<VideoBuffer> SaveRenderer::Render(const GameSave *save, bool fire, RendererSettings rendererSettings)
{
	// this function usually runs on a thread different from where element info in SimulationData may be written, so we acquire a read-only lock on it
	auto &sd = SimulationData::CRef();
	std::shared_lock lk(sd.elementGraphicsMx);
	std::lock_guard<std::mutex> gx(renderMutex);

	ren->ApplySettings(rendererSettings);

	sim->clear_sim();

	sim->Load(save, true, { 0, 0 });
	ren->ClearAccumulation();
	ren->Clear();
	if (fire)
	{
		ren->ApproximateAccumulation();
	}
	ren->RenderSimulation();

	auto tempThumb = std::make_unique<VideoBuffer>(save->blockSize * CELL);
	auto &video = ren->GetVideo();
	tempThumb->BlendImage(video.data(), 0xFF, video.Size().OriginRect());

	return tempThumb;
}
