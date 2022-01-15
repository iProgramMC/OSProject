/*****************************************
		NanoShell Operating System
		  (C) 2022 iProgramInCpp

           Shell widget module
******************************************/
#include <widget.h>
#include <video.h>
#include <icon.h>

// Utilitary functions
#if 1
/***************************************************************************
	Explanation of how this is supposed to render:

	LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
	LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLD
	LLWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWDD
	LLWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWDD
	LLWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWDD
	LLWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWDD
	LLWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWDD
	LLWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWDD
	LLWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWDD
	LDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
	DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
***************************************************************************/
void RenderButtonShape(Rectangle rect, unsigned colorDark, unsigned colorLight, unsigned colorMiddle)
{
	//draw some lines
	VidDrawHLine (colorLight, rect.left+1,rect.right-1,  rect.top);
	VidDrawHLine (colorDark,  rect.left+1,rect.right-1,  rect.bottom-1);
	VidDrawVLine (colorLight, rect.top+1, rect.bottom-3, rect.left);
	VidDrawVLine (colorDark,  rect.top+2, rect.bottom-2, rect.right);
	
	//shrink
	rect.left++, rect.right--, rect.top++, rect.bottom--;
	
	//do the same
	VidDrawHLine (colorLight, rect.left,  rect.right,    rect.top);
	VidDrawHLine (colorDark,  rect.left,  rect.right,    rect.bottom-1);
	VidDrawVLine (colorLight, rect.top,   rect.bottom-2, rect.left);
	VidDrawVLine (colorDark,  rect.top+1, rect.bottom-1, rect.right);
	
	//shrink again
	rect.left++, rect.right--, rect.top++, rect.bottom -= 2;
	
	//fill the background:
	if (colorMiddle != TRANSPARENT)
		VidFillRectangle(colorMiddle, rect);
	
	//then, the corners.
	VidPlotPixel(rect.left,  rect.top, 	  colorLight);
	VidPlotPixel(rect.left,  rect.bottom, colorDark);
	VidPlotPixel(rect.right, rect.top,	  colorLight);
	VidPlotPixel(rect.right, rect.bottom, colorDark);
}
void RenderButtonShapeNoRounding(Rectangle rect, unsigned colorDark, unsigned colorLight, unsigned colorMiddle)
{
	//draw some lines
	VidDrawVLine (colorLight, rect.top,   rect.bottom-1,   rect.left);
	VidDrawVLine (colorDark,  rect.top,   rect.bottom-1,   rect.right);
	VidDrawHLine (colorDark,  rect.left,  rect.right,      rect.bottom - 1);
	VidDrawHLine (colorLight, rect.left,  rect.right,      rect.top);
	
	//shrink
	rect.left++, rect.right--, rect.top++, rect.bottom--;
	
	//do the same
	VidDrawVLine (colorLight, rect.top,   rect.bottom-1,   rect.left);
	VidDrawVLine (colorDark,  rect.top,   rect.bottom-1,   rect.right);
	VidDrawHLine (colorDark,  rect.left,  rect.right,      rect.bottom - 1);
	VidDrawHLine (colorLight, rect.left,  rect.right,      rect.top);
	
	//shrink again
	rect.left++, rect.right--, rect.top++, rect.bottom -= 2;
	
	//fill the background:
	if (colorMiddle != TRANSPARENT)
		VidFillRectangle(colorMiddle, rect);
}
void RenderButtonShapeSmall(Rectangle rect, unsigned colorDark, unsigned colorLight, unsigned colorMiddle)
{
	//draw some lines
	VidDrawVLine (colorLight, rect.top,   rect.bottom-1,   rect.left);
	VidDrawVLine (colorDark,  rect.top,   rect.bottom-1,   rect.right);
	VidDrawHLine (colorDark,  rect.left,  rect.right,      rect.bottom - 1);
	VidDrawHLine (colorLight, rect.left,  rect.right,      rect.top);
	
	//shrink
	rect.left++, rect.right--, rect.top++, rect.bottom -= 2;
	
	//fill the background:
	if (colorMiddle != TRANSPARENT)
		VidFillRectangle(colorMiddle, rect);
}
#endif

// List View.
#if 1

static void CtlAddElementToList (Control* pCtl, const char* pText, int optionalIcon)
{
	ListViewData* pData = &pCtl->m_listViewData;
	if (pData->m_elementCount == pData->m_capacity)
	{
		//have to expand first
		int oldSize = sizeof (ListItem) * pData->m_capacity;
		int newSize = oldSize * 2;
		LogMsg("Expanding capacity from %d to %d (in bytes)", oldSize, newSize);
		ListItem* pNewItems = MmAllocate(newSize);
		ZeroMemory(pNewItems, newSize);
		memcpy (pNewItems, pData->m_pItems, oldSize);
		MmFree (pData->m_pItems);
		pData->m_pItems = pNewItems;
		pData->m_capacity *= 2;
		
		//then can add
	}
	ListItem *pItem = &pData->m_pItems[pData->m_elementCount];
	pData->m_elementCount++;
	
	pItem->m_icon = optionalIcon;
	strcpy(pItem->m_contents, pText);
}

static void CtlRemoveElementFromList(Control* pCtl, int index)
{
	ListViewData* pData = &pCtl->m_listViewData;
	memcpy (pData->m_pItems + index, pData->m_pItems + index + 1, sizeof(ListItem) * (pData->m_elementCount - index - 1));
	pData->m_elementCount--;
}

static void CtlResetList (Control* pCtl)
{
	ListViewData* pData = &pCtl->m_listViewData;
	
	if (!pData) return;//TODO
	pData->m_elementCount = 0;
	pData->m_capacity     = 10;
	pData->m_hasIcons     = false;
	int itemsSize         = sizeof (ListItem) * pData->m_capacity;
	pData->m_pItems       = MmAllocate (itemsSize);
	memset (pData->m_pItems, 0, itemsSize);
}

void AddElementToList (Window* pWindow, int comboID, const char* pText, int optionalIcon)
{
	for (int i = 0; i < pWindow->m_controlArrayLen; i++)
	{
		if (pWindow->m_pControlArray[i].m_comboID == comboID)
		{
			CtlAddElementToList (&pWindow->m_pControlArray[i], pText, optionalIcon);
			return;
		}
	}
}

void RemoveElementFromList (Window* pWindow, int comboID, int elementIndex)
{
	for (int i = 0; i < pWindow->m_controlArrayLen; i++)
	{
		if (pWindow->m_pControlArray[i].m_comboID == comboID)
		{
			CtlRemoveElementFromList (&pWindow->m_pControlArray[i], elementIndex);
			return;
		}
	}
}

void ResetList (Window* pWindow, int comboID)
{
	for (int i = 0; i < pWindow->m_controlArrayLen; i++)
	{
		if (pWindow->m_pControlArray[i].m_comboID == comboID)
		{
			CtlResetList (&pWindow->m_pControlArray[i]);
			return;
		}
	}
}
extern VBEData*g_vbeData,g_mainScreenVBEData;
void WidgetListView_OnEvent(Control* this, UNUSED int eventType, UNUSED int parm1, UNUSED int parm2, UNUSED Window* pWindow)
{
	switch (eventType)
	{
	#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
		case EVENT_CLICKCURSOR:
		{
			CtlAddElementToList(this, "clicking text goes here", ICON_FOLDER16_OPEN);
		}
	#pragma GCC diagnostic pop
		case EVENT_PAINT:
		{
			//draw a green rectangle:
			VidFillRectangle(0xFFFFFF, this->m_rect);
			ListViewData* pData = &this->m_listViewData;
			
			int elementStart =   pData->m_scrollY / LIST_ITEM_HEIGHT;
			int elementEnd   =  (pData->m_scrollY + this->m_rect.bottom - this->m_rect.top) / LIST_ITEM_HEIGHT;
			int offsetY      = -(pData->m_scrollY % LIST_ITEM_HEIGHT);
			
			if (elementStart < 0) elementStart = 0;
			if (elementStart >= pData->m_elementCount) elementStart = pData->m_elementCount-1;
			
			if (elementEnd < 0) elementEnd = 0;
			if (elementEnd >= pData->m_elementCount) elementEnd = pData->m_elementCount-1;
			
			VBEData*ppData=g_vbeData;g_vbeData=&g_mainScreenVBEData;
			LogMsg("elementStart=%d elementEnd=%d offsetY=%d pData->m_elementCount=%d pData->m_scrollY=%d", elementStart, elementEnd, offsetY, pData->m_elementCount, pData->m_scrollY);
			g_vbeData=ppData;
			
			if (elementStart > elementEnd)
				VidDrawText ("(Empty)", this->m_rect, TEXTSTYLE_HCENTERED, 0x7F7F7F, TRANSPARENT);
			for (int i = elementStart; i <= elementEnd; i++)
			{
				if (pData->m_hasIcons)
				{
					if (pData->m_pItems[i].m_icon)
						RenderIcon (pData->m_pItems[i].m_icon, this->m_rect.left + 4, this->m_rect.top + 2 + i * LIST_ITEM_HEIGHT - offsetY);
				}
				VidTextOut (pData->m_pItems[i].m_contents, this->m_rect.left + 4 + pData->m_hasIcons * 16, this->m_rect.top + 4 + 2 + i * LIST_ITEM_HEIGHT - offsetY, 0, TRANSPARENT);
			}
			
			RenderButtonShapeNoRounding (this->m_rect, 0xBFBFBF, 0x000000, TRANSPARENT);
			
			break;
		}
		case EVENT_CREATE:
		{
			// Start out with an initial size of 10 elements.
			ListViewData* pData = &this->m_listViewData;
			pData->m_elementCount = 0;
			pData->m_capacity     = 10;
			pData->m_scrollY      = 0;
			pData->m_hasIcons     = true;
			int itemsSize         = sizeof (ListItem) * pData->m_capacity;
			pData->m_pItems       = MmAllocate (itemsSize);
			memset (pData->m_pItems, 0, itemsSize);
			break;
		}
		case EVENT_DESTROY:
		{
			ListViewData* pData = &this->m_listViewData;
			//free the items first
			if (pData->m_pItems)
			{
				MmFree (pData->m_pItems);
				pData->m_pItems = NULL;
			}
			//also free the pData itself.
			MmFree (pData);
			this->m_dataPtr = NULL;
			break;
		}
	}
}
#endif

// Basic controls
#if 1
void WidgetNone_OnEvent(UNUSED Control* this, UNUSED int eventType, UNUSED int parm1, UNUSED int parm2, UNUSED Window* pWindow)
{
}
void WidgetText_OnEvent(UNUSED Control* this, UNUSED int eventType, UNUSED int parm1, UNUSED int parm2, UNUSED Window* pWindow)
{
	switch (eventType)
	{
		case EVENT_PAINT:
			VidTextOut(this->m_text, this->m_rect.left, this->m_rect.top, this->m_parm1, this->m_parm2);
			break;
	}
}
void WidgetTextCenter_OnEvent(UNUSED Control* this, UNUSED int eventType, UNUSED int parm1, UNUSED int parm2, UNUSED Window* pWindow)
{
	switch (eventType)
	{
		case EVENT_PAINT:
			VidDrawText(this->m_text, this->m_rect, this->m_parm2, this->m_parm1, TRANSPARENT);
			break;
	}
}
void WidgetIcon_OnEvent(UNUSED Control* this, UNUSED int eventType, UNUSED int parm1, UNUSED int parm2, UNUSED Window* pWindow)
{
	switch (eventType)
	{
		case EVENT_PAINT:
			RenderIcon(this->m_parm1, this->m_rect.left + (this->m_rect.right - this->m_rect.left - 32) / 2, this->m_rect.top + (this->m_rect.bottom - this->m_rect.top - 32) / 2);
			break;
	}
}
void WidgetButton_OnEvent(UNUSED Control* this, UNUSED int eventType, UNUSED int parm1, UNUSED int parm2, UNUSED Window* pWindow)
{
	switch (eventType)
	{
	#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
		case EVENT_RELEASECURSOR:
		{
			Rectangle r = this->m_rect;
			Point p = { GET_X_PARM(parm1), GET_Y_PARM(parm1) };
			if (RectangleContains (&r, &p))
			{
				//send a command event to the window:
				//WindowRegisterEvent(pWindow, EVENT_COMMAND, this->m_parm1, this->m_parm2);
				pWindow->m_callback (pWindow, EVENT_COMMAND, this->m_comboID, this->m_parm1);
			}
		}
		//! fallthrough intentional - need the button to redraw itself as pushing back up
		case EVENT_PAINT:
	#pragma GCC diagnostic pop
		{
			//draw a green rectangle:
			//VidFillRectangle(0xFF00, this->m_rect);
			RenderButtonShape (this->m_rect, 0x000000, 0xFFFFFF, BUTTON_MIDDLE_COLOR);
			//then fill in the text:
			VidDrawText(this->m_text, this->m_rect, TEXTSTYLE_HCENTERED|TEXTSTYLE_VCENTERED, 0, TRANSPARENT);
			
			break;
		}
		case EVENT_CLICKCURSOR:
		{
			Rectangle r = this->m_rect;
			Point p = { GET_X_PARM(parm1), GET_Y_PARM(parm1) };
			if (RectangleContains (&r, &p))
			{
				//draw the button as slightly pushed in
				r.left++; r.right++; r.bottom++; r.top++;
				
				RenderButtonShape (this->m_rect, 0xFFFFFF, 0x000000, WINDOW_BACKGD_COLOR);
				//then fill in the text:
				VidDrawText(this->m_text, this->m_rect, TEXTSTYLE_HCENTERED|TEXTSTYLE_VCENTERED, 0, TRANSPARENT);
			}
			break;
		}
	}
}
//for the top bar of the window.  Uses this->m_parm1 as the event type.
void WidgetActionButton_OnEvent(UNUSED Control* this, UNUSED int eventType, UNUSED int parm1, UNUSED int parm2, UNUSED Window* pWindow)
{
	switch (eventType)
	{
	#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
		case EVENT_RELEASECURSOR:
		{
			Rectangle r = this->m_rect;
			Point p = { GET_X_PARM(parm1), GET_Y_PARM(parm1) };
			if (RectangleContains (&r, &p))
			{
				//send a command event to the window:
				//WindowRegisterEvent(pWindow, EVENT_COMMAND, this->m_parm1, this->m_parm2);
				pWindow->m_callback (pWindow, this->m_parm1, this->m_comboID, this->m_parm2);
			}
		}
		//! fallthrough intentional - need the button to redraw itself as pushing back up
		case EVENT_PAINT:
	#pragma GCC diagnostic pop
		{
			//draw a green rectangle:
			//VidFillRectangle(0xFF00, this->m_rect);
			RenderButtonShapeSmall (this->m_rect, 0x000000, 0xFFFFFF, BUTTON_MIDDLE_COLOR);
			//then fill in the text:
			VidDrawText(this->m_text, this->m_rect, TEXTSTYLE_HCENTERED|TEXTSTYLE_VCENTERED, 0, TRANSPARENT);
			
			break;
		}
		case EVENT_CLICKCURSOR:
		{
			Rectangle r = this->m_rect;
			Point p = { GET_X_PARM(parm1), GET_Y_PARM(parm1) };
			if (RectangleContains (&r, &p))
			{
				//draw the button as slightly pushed in
				r.left++; r.right++; r.bottom++; r.top++;
				
				RenderButtonShapeSmall (this->m_rect, 0xFFFFFF, 0x000000, WINDOW_BACKGD_COLOR);
				//then fill in the text:
				VidDrawText(this->m_text, this->m_rect, TEXTSTYLE_HCENTERED|TEXTSTYLE_VCENTERED, 0, TRANSPARENT);
			}
			break;
		}
	}
}
void WidgetClickLabel_OnEvent(UNUSED Control* this, UNUSED int eventType, UNUSED int parm1, UNUSED int parm2, UNUSED Window* pWindow)
{
	switch (eventType)
	{
	#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
		case EVENT_RELEASECURSOR:
		{
			Rectangle r = this->m_rect;
			Point p = { GET_X_PARM(parm1), GET_Y_PARM(parm1) };
			if (RectangleContains (&r, &p))
			{
				//send a command event to the window:
				//WindowRegisterEvent(pWindow, EVENT_COMMAND, this->m_parm1, this->m_parm2);
				pWindow->m_callback (pWindow, EVENT_COMMAND, this->m_comboID, this->m_parm1);
			}
		}
		//! fallthrough intentional - need the button to redraw itself as pushing back up
		case EVENT_PAINT:
	#pragma GCC diagnostic pop
		{
			//then fill in the text:
			VidDrawText(this->m_text, this->m_rect, TEXTSTYLE_VCENTERED, 0x1111FF, TRANSPARENT);
			
			break;
		}
		case EVENT_CLICKCURSOR:
		{
			Rectangle r = this->m_rect;
			Point p = { GET_X_PARM(parm1), GET_Y_PARM(parm1) };
			if (RectangleContains (&r, &p))
			{
				//then fill in the text:
				VidDrawText(this->m_text, this->m_rect, TEXTSTYLE_VCENTERED, 0x11, TRANSPARENT);
			}
			break;
		}
	}
}
void WidgetTextInput_OnEvent(UNUSED Control* this, UNUSED int eventType, UNUSED int parm1, UNUSED int parm2, UNUSED Window* pWindow)
{
}
void WidgetCheckbox_OnEvent(UNUSED Control* this, UNUSED int eventType, UNUSED int parm1, UNUSED int parm2, UNUSED Window* pWindow)
{
}
#define SCROLL_BAR_WIDTH 16
void WidgetHScrollBar_OnEvent(UNUSED Control* this, UNUSED int eventType, UNUSED int parm1, UNUSED int parm2, UNUSED Window* pWindow)
{
	Rectangle basic_rectangle = this->m_rect;
	basic_rectangle.bottom = basic_rectangle.top + SCROLL_BAR_WIDTH-1;
	
	Rectangle left_button = basic_rectangle;
	left_button.right = left_button.left + SCROLL_BAR_WIDTH;
	
	Rectangle right_button = basic_rectangle;
	right_button.left = right_button.right - SCROLL_BAR_WIDTH;
	
	Rectangle clickable_rect = basic_rectangle;
	clickable_rect.left  = left_button.right;
	clickable_rect.right = right_button.left;
	
	int height = clickable_rect.right - clickable_rect.left;
	int final_height = height - SCROLL_BAR_WIDTH;
		
	int offset = this->m_scrollBarData.m_pos - this->m_scrollBarData.m_min;
	int posoff = offset * final_height / (this->m_scrollBarData.m_max-1 - this->m_scrollBarData.m_min);
	
	Rectangle scroller = basic_rectangle;
	scroller.left  = clickable_rect.left + posoff;
	scroller.right = clickable_rect.left + posoff + SCROLL_BAR_WIDTH;
	
	bool paint = true;
	switch (eventType)
	{
		case EVENT_CLICKCURSOR:
		{
			Point p = { GET_X_PARM(parm1), GET_Y_PARM(parm1) };
			if (RectangleContains(&scroller, &p))
			{
				this->m_scrollBarData.m_isBeingDragged = true;
			}
			if (RectangleContains(&left_button, &p))
			{
				if (!this->m_scrollBarData.m_clickedBefore)
				{
					this->m_scrollBarData.m_yMinButton = true;
					this->m_scrollBarData.m_pos -= (this->m_scrollBarData.m_max - this->m_scrollBarData.m_min) / 10;
					if (this->m_scrollBarData.m_pos < this->m_scrollBarData.m_min)
						this->m_scrollBarData.m_pos = this->m_scrollBarData.m_min;
				}
			}
			if (RectangleContains(&right_button, &p))
			{
				if (!this->m_scrollBarData.m_clickedBefore)
				{
					this->m_scrollBarData.m_yMaxButton = true;
					this->m_scrollBarData.m_pos += (this->m_scrollBarData.m_max - this->m_scrollBarData.m_min) / 10;
					if (this->m_scrollBarData.m_pos >= this->m_scrollBarData.m_max)
						this->m_scrollBarData.m_pos  = this->m_scrollBarData.m_max - 1;
				}
			}
			
			if (this->m_scrollBarData.m_isBeingDragged)
			{
				int posoff2 = p.x - clickable_rect.left - SCROLL_BAR_WIDTH/2;
				posoff2 = posoff2 * (this->m_scrollBarData.m_max-1 - this->m_scrollBarData.m_min) / final_height;
				posoff2 = posoff2 +  this->m_scrollBarData.m_min;
				if (posoff2 <  this->m_scrollBarData.m_min) posoff2 = this->m_scrollBarData.m_min;
				if (posoff2 >= this->m_scrollBarData.m_max) posoff2 = this->m_scrollBarData.m_max - 1;
				this->m_scrollBarData.m_pos = posoff2;
			}
			
			this->m_scrollBarData.m_clickedBefore = true;
			break;
		}
		case EVENT_RELEASECURSOR:
		{
			this->m_scrollBarData.m_isBeingDragged = false;
			this->m_scrollBarData.m_clickedBefore  = false;
			this->m_scrollBarData.m_yMinButton     = false;
			this->m_scrollBarData.m_yMaxButton     = false;
			break;
		}
		case EVENT_PAINT:
		{
			break;
		}
	}
	if (paint)
	{
		VidFillRectangle (0x7F7F7F, basic_rectangle);
		
		if (this->m_scrollBarData.m_yMinButton)
			RenderButtonShapeNoRounding (left_button,   0xFFFFFF, 0x000000, WINDOW_BACKGD_COLOR);
		else
			RenderButtonShapeNoRounding (left_button,   0x000000, 0xFFFFFF, BUTTON_MIDDLE_COLOR);
		
		if (this->m_scrollBarData.m_yMaxButton)
			RenderButtonShapeNoRounding (right_button,  0xFFFFFF, 0x000000, WINDOW_BACKGD_COLOR);
		else
			RenderButtonShapeNoRounding (right_button,  0x000000, 0xFFFFFF, BUTTON_MIDDLE_COLOR);
		
		RenderButtonShapeNoRounding (scroller, 0x000000, 0xFFFFFF, BUTTON_MIDDLE_COLOR + this->m_scrollBarData.m_isBeingDragged * 0x222222); // Green
		
		left_button .left++; left_button .right++; left_button .bottom++; left_button .top++;
		right_button.left++; right_button.right++; right_button.bottom++; right_button.top++;
		scroller    .left++; scroller    .right++; scroller    .bottom++; scroller    .top++;
		
		VidDrawText ("\x1B",   left_button,  TEXTSTYLE_HCENTERED|TEXTSTYLE_VCENTERED, 0, TRANSPARENT);
		VidDrawText ("\x1A",   right_button, TEXTSTYLE_HCENTERED|TEXTSTYLE_VCENTERED, 0, TRANSPARENT);
		VidDrawText ("\x12",   scroller,     TEXTSTYLE_HCENTERED|TEXTSTYLE_VCENTERED, 0, TRANSPARENT);
	}
}
void WidgetVScrollBar_OnEvent(UNUSED Control* this, UNUSED int eventType, UNUSED int parm1, UNUSED int parm2, UNUSED Window* pWindow)
{
	Rectangle basic_rectangle = this->m_rect;
	basic_rectangle.right = basic_rectangle.left + SCROLL_BAR_WIDTH-1;
	
	Rectangle top_button = basic_rectangle;
	top_button.bottom = top_button.top + SCROLL_BAR_WIDTH;
	
	Rectangle bottom_button = basic_rectangle;
	bottom_button.top = bottom_button.bottom - SCROLL_BAR_WIDTH;
	
	Rectangle clickable_rect = basic_rectangle;
	clickable_rect.top    = top_button.bottom;
	clickable_rect.bottom = bottom_button.top;
	
	int height = clickable_rect.bottom - clickable_rect.top;
	int final_height = height - SCROLL_BAR_WIDTH;
		
	int offset = this->m_scrollBarData.m_pos - this->m_scrollBarData.m_min;
	int posoff = offset * final_height / (this->m_scrollBarData.m_max-1 - this->m_scrollBarData.m_min);
	
	Rectangle scroller = basic_rectangle;
	scroller.top    = clickable_rect.top + posoff;
	scroller.bottom = clickable_rect.top + posoff + SCROLL_BAR_WIDTH;
	
	bool paint = true;
	switch (eventType)
	{
		case EVENT_CLICKCURSOR:
		{
			Point p = { GET_X_PARM(parm1), GET_Y_PARM(parm1) };
			if (RectangleContains(&scroller, &p))
			{
				this->m_scrollBarData.m_isBeingDragged = true;
			}
			if (RectangleContains(&top_button, &p))
			{
				if (!this->m_scrollBarData.m_clickedBefore)
				{
					this->m_scrollBarData.m_yMinButton = true;
					this->m_scrollBarData.m_pos -= (this->m_scrollBarData.m_max - this->m_scrollBarData.m_min) / 10;
					if (this->m_scrollBarData.m_pos < this->m_scrollBarData.m_min)
						this->m_scrollBarData.m_pos = this->m_scrollBarData.m_min;
				}
			}
			if (RectangleContains(&bottom_button, &p))
			{
				if (!this->m_scrollBarData.m_clickedBefore)
				{
					this->m_scrollBarData.m_yMaxButton = true;
					this->m_scrollBarData.m_pos += (this->m_scrollBarData.m_max - this->m_scrollBarData.m_min) / 10;
					if (this->m_scrollBarData.m_pos >= this->m_scrollBarData.m_max)
						this->m_scrollBarData.m_pos  = this->m_scrollBarData.m_max - 1;
				}
			}
			
			if (this->m_scrollBarData.m_isBeingDragged)
			{
				int posoff2 = p.y - clickable_rect.top - SCROLL_BAR_WIDTH/2;
				posoff2 = posoff2 * (this->m_scrollBarData.m_max-1 - this->m_scrollBarData.m_min) / final_height;
				posoff2 = posoff2 +  this->m_scrollBarData.m_min;
				if (posoff2 <  this->m_scrollBarData.m_min) posoff2 = this->m_scrollBarData.m_min;
				if (posoff2 >= this->m_scrollBarData.m_max) posoff2 = this->m_scrollBarData.m_max - 1;
				this->m_scrollBarData.m_pos = posoff2;
			}
			
			this->m_scrollBarData.m_clickedBefore = true;
			break;
		}
		case EVENT_RELEASECURSOR:
		{
			this->m_scrollBarData.m_isBeingDragged = false;
			this->m_scrollBarData.m_clickedBefore  = false;
			this->m_scrollBarData.m_yMinButton     = false;
			this->m_scrollBarData.m_yMaxButton     = false;
			break;
		}
		case EVENT_PAINT:
		{
			break;
		}
	}
	if (paint)
	{
		VidFillRectangle (0x7F7F7F, basic_rectangle);
		if (this->m_scrollBarData.m_yMinButton)
			RenderButtonShapeNoRounding (top_button,     0xFFFFFF, 0x000000, WINDOW_BACKGD_COLOR);
		else
			RenderButtonShapeNoRounding (top_button,     0x000000, 0xFFFFFF, BUTTON_MIDDLE_COLOR);
		
		if (this->m_scrollBarData.m_yMaxButton)
			RenderButtonShapeNoRounding (bottom_button,  0xFFFFFF, 0x000000, WINDOW_BACKGD_COLOR);
		else
			RenderButtonShapeNoRounding (bottom_button,  0x000000, 0xFFFFFF, BUTTON_MIDDLE_COLOR);
		
		RenderButtonShapeNoRounding (scroller, 0x000000, 0xFFFFFF, BUTTON_MIDDLE_COLOR + this->m_scrollBarData.m_isBeingDragged * 0x222222); // Green
		
		top_button   .left++; top_button   .right++; top_button   .bottom++; top_button   .top++;
		bottom_button.left++; bottom_button.right++; bottom_button.bottom++; bottom_button.top++;
		scroller     .left++; scroller     .right++; scroller     .bottom++; scroller     .top++;
		
		VidDrawText ("\x18",   top_button,    TEXTSTYLE_HCENTERED|TEXTSTYLE_VCENTERED, 0, TRANSPARENT);
		VidDrawText ("\x19",   bottom_button, TEXTSTYLE_HCENTERED|TEXTSTYLE_VCENTERED, 0, TRANSPARENT);
		VidDrawText ("\x12",   scroller,      TEXTSTYLE_HCENTERED|TEXTSTYLE_VCENTERED, 0, TRANSPARENT);
	}
}
#endif

WidgetEventHandler g_widgetEventHandlerLUT[] = {
	WidgetNone_OnEvent,
	WidgetText_OnEvent,
	WidgetIcon_OnEvent,
	WidgetButton_OnEvent,
	WidgetTextInput_OnEvent,
	WidgetCheckbox_OnEvent,
	WidgetClickLabel_OnEvent,
	WidgetTextCenter_OnEvent,
	WidgetActionButton_OnEvent,
	WidgetListView_OnEvent,
	WidgetVScrollBar_OnEvent,
	WidgetHScrollBar_OnEvent,
	NULL
};
WidgetEventHandler GetWidgetOnEventFunction (int type)
{
	if (type < 0 || type >= CONTROL_COUNT)
		return NULL;
	return g_widgetEventHandlerLUT[type];
}
