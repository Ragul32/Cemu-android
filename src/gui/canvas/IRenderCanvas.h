#pragma once

#include <wx/wxprec.h>
#include "gui/guiWrapper.h"
// base class for all render interfaces
class IRenderCanvas
{
public:
	IRenderCanvas(bool is_main_window)
		: m_is_main_window(is_main_window) 
	{
		if(m_is_main_window)
			gui_getMainCanvasSize.connect(std::bind(&IRenderCanvas::GetCanvasSize, this, std::placeholders::_1, std::placeholders::_2));
		else
			gui_getPadCanvasSize.connect(std::bind(&IRenderCanvas::GetCanvasSize, this, std::placeholders::_1, std::placeholders::_2));
	}

	virtual ~IRenderCanvas()
	{
		if(m_is_main_window)
			gui_getMainCanvasSize.disconnect_all_slots();
		else
			gui_getPadCanvasSize.disconnect_all_slots();
	}
protected:
	virtual void GetCanvasSize(int &width, int &height) = 0;
	bool m_is_main_window;
};