#include "gui/canvas/VulkanCanvas.h"
#include "Cafe/HW/Latte/Renderer/Vulkan/VulkanRenderer.h"
#include "gui/guiWrapper.h"

#include <wx/msgdlg.h>

VulkanCanvas::VulkanCanvas(wxWindow* parent, const wxSize& size, bool is_main_window)
	: IRenderCanvas(is_main_window), wxWindow(parent, wxID_ANY, wxDefaultPosition, size, wxNO_FULL_REPAINT_ON_RESIZE | wxWANTS_CHARS)
{
	Bind(wxEVT_PAINT, &VulkanCanvas::OnPaint, this);
	Bind(wxEVT_SIZE, &VulkanCanvas::OnResize, this);

	WindowHandleInfo *canvas;
	if(is_main_window)
	{
		canvas = &gui_getWindowInfo().canvas_main;

	}
	else
		canvas = &gui_getWindowInfo().canvas_pad;

	gui_initHandleContextFromWxWidgetsWindow(*canvas, this);
	#if BOOST_OS_LINUX
	if(canvas->backend == WindowHandleInfo::Backend::WAYLAND)
	{	
		m_subsurface = std::make_unique<wxWlSubsurface>(this);
		canvas->surface = m_subsurface->getSurface();
	}
	#endif
	cemu_assert(g_vulkan_available);

	try
	{
		if (is_main_window)
			g_renderer = std::make_unique<VulkanRenderer>();

		auto vulkan_renderer = VulkanRenderer::GetInstance();
		vulkan_renderer->InitializeSurface({size.x, size.y}, is_main_window);
	}
	catch(const std::exception& ex)
	{
		const auto msg = fmt::format(fmt::runtime(_("Error when initializing Vulkan renderer:\n{}").ToStdString()), ex.what());
		forceLog_printf(const_cast<char*>(msg.c_str()));
		wxMessageDialog dialog(this, msg, _("Error"), wxOK | wxCENTRE | wxICON_ERROR);
		dialog.ShowModal();
		exit(0);
	}

	wxWindow::EnableTouchEvents(wxTOUCH_PAN_GESTURES);
}

VulkanCanvas::~VulkanCanvas()
{
	Unbind(wxEVT_PAINT, &VulkanCanvas::OnPaint, this);
	Unbind(wxEVT_SIZE, &VulkanCanvas::OnResize, this);

	if(!m_is_main_window)
	{
		if(auto vulkan_renderer = VulkanRenderer::GetInstance())
			vulkan_renderer->StopUsingPadAndWait();
	}
}

void VulkanCanvas::OnPaint(wxPaintEvent& event)
{
}

void VulkanCanvas::OnResize(wxSizeEvent& event)
{
#if BOOST_OS_LINUX
	if(m_subsurface)
	{
		int32_t x,y;
		GetScreenPosition(&x,&y);
		m_subsurface->setPosition(x, y);
	}
#endif
	const wxSize size = GetSize();
	if (size.GetWidth() == 0 || size.GetHeight() == 0)
		return;

	const wxRect refreshRect(size);
	RefreshRect(refreshRect, false);
}
