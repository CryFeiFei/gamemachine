﻿#include "stdafx.h"
#include "gmcontrols.h"
#include "gmwidget.h"
#include "foundation/gamemachine.h"
#include "gmcontrols_p.h"

BEGIN_NS

GMControl::GMControl(GMWidget* widget)
{
	GM_CREATE_DATA();

	D(d);
	d->widget = widget;
}

GMControl::~GMControl()
{
}

void GMControl::setVisible(bool visible)
{
	D(d);
	d->visible = visible;
}

bool GMControl::getEnabled()
{
	D(d);
	return d->enabled;
}

void GMControl::setEnabled(bool enabled)
{
	D(d);
	d->enabled = enabled;
}

bool GMControl::handleSystemEvent(GMSystemEvent* event)
{
	return false;
}

bool GMControl::getVisible()
{
	D(d);
	return d->visible;
}

bool GMControl::getMouseOver()
{
	D(d);
	return d->mouseOver;
}

bool GMControl::hasFocus()
{
	D(d);
	return d->hasFocus;
}

bool GMControl::isDefault() GM_NOEXCEPT
{
	D(d);
	return d->isDefault;
}

GMWidget* GMControl::getParent() GM_NOEXCEPT
{
	D(d);
	return d->widget;
}

GMint32 GMControl::getIndex() GM_NOEXCEPT
{
	D(d);
	return d->index;
}

const GMRect& GMControl::getBoundingRect() GM_NOEXCEPT
{
	D(d);
	return d->boundingBox;
}

GMint32 GMControl::getWidth() GM_NOEXCEPT
{
	D(d);
	return d->width;
}

GMint32 GMControl::getHeight() GM_NOEXCEPT
{
	D(d);
	return d->height;
}

GMPoint GMControl::toControlSpace(const GMPoint& rc) GM_NOEXCEPT
{
	D(d);
	GMPoint r = { rc.x - d->boundingBox.x, rc.y - d->boundingBox.y };
	return r;
}

void GMControl::setPositionFlag(GMControlPositionFlag flag) GM_NOEXCEPT
{
	D(d);
	if (d->positionFlag != flag)
		d->positionFlag = flag;
}

GMControlPositionFlag GMControl::getPositionFlag() const GM_NOEXCEPT
{
	D(d);
	return d->positionFlag;
}

bool GMControl::handleKeyboard(GMSystemKeyEvent* event)
{
	bool handled = false;
	switch (event->getType())
	{
	case GMSystemEventType::KeyDown:
		handled = onKeyDown(event);
		break;
	case GMSystemEventType::KeyUp:
		handled = onKeyUp(event);
		break;
	case GMSystemEventType::Char:
		handled = onChar(static_cast<GMSystemCharEvent*>(event));
		break;
	}
	return handled;
}

bool GMControl::handleMouse(GMSystemMouseEvent* event)
{
	bool handled = false;
	switch (event->getType())
	{
	case GMSystemEventType::MouseDown:
		handled = onMouseDown(event);
		break;
	case GMSystemEventType::MouseUp:
		handled = onMouseUp(event);
		break;
	case GMSystemEventType::MouseMove:
		handled = onMouseMove(event);
		break;
	case GMSystemEventType::MouseDblClick:
		handled = onMouseDblClick(event);
		break;
	case GMSystemEventType::MouseWheel:
		handled = onMouseWheel(static_cast<GMSystemMouseWheelEvent*>(event));
		break;
	}
	return handled;
}

void GMControl::updateRect()
{
	D(d);
	d->boundingBox.x = d->x;
	d->boundingBox.y = d->y;
	d->boundingBox.width = d->width;
	d->boundingBox.height = d->height;
}

void GMControl::onFocusIn()
{
	D(d);
	d->hasFocus = true;
}

void GMControl::onFocusOut()
{
	D(d);
	d->hasFocus = false;
}

void GMControl::onMouseEnter()
{
	D(d);
	d->mouseOver = true;
}

void GMControl::onMouseLeave()
{
	D(d);
	d->mouseOver = false;
}

void GMControl::refresh()
{
	D(d);
	d->mouseOver = false;
	d->hasFocus = false;
}

GMStyle& GMControl::getStyle(StyleType style)
{
	static GMStyle s_style;
	return s_style;
}

bool GMControl::containsPoint(GMPoint point)
{
	D(d);
	return GM_inRect(d->boundingBox, point);
}

void GMControl::setIsDefault(bool isDefault)
{
	D(d);
	d->isDefault = isDefault;
}

void GMControl::setSize(GMint32 width, GMint32 height)
{
	D(d);
	if (d->width != width || d->height != height)
	{
		d->width = width;
		d->height = height;
		updateRect();
	}
}

void GMControl::setPosition(GMint32 x, GMint32 y)
{
	D(d);
	if (d->x != x || d->y != y)
	{
		d->x = x;
		d->y = y;
		updateRect();
	}
}

void GMControl::setIndex(GMint32 index)
{
	D(d);
	d->index = index;
}

GMControlLabel* GMControlLabel::createControl(
	GMWidget* widget,
	const GMString& text,
	const GMVec4& fontColor,
	GMint32 x,
	GMint32 y,
	GMint32 width,
	GMint32 height,
	bool isDefault
)
{
	GMControlLabel* labelControl = new GMControlLabel(widget);
	labelControl->setText(text);
	labelControl->setPosition(x, y);
	labelControl->setSize(width, height);
	labelControl->setFontColor(fontColor);
	labelControl->setIsDefault(isDefault);
	return labelControl;
}

GMControlLabel::GMControlLabel(GMWidget* widget) : Base(widget)
{
	GM_CREATE_DATA();
	initStyles(widget);
}

GMControlLabel::~GMControlLabel()
{

}

void GMControlLabel::render(GMDuration elapsed)
{
	if (!getVisible())
		return;

	D(d);
	D_BASE(db, Base);

	GMControlState::State state = GMControlState::Normal;
	if (!getEnabled())
		state = GMControlState::Disabled;

	GMStyle& foreStyle = getStyle((GMControl::StyleType)ForeStyle);
	foreStyle.getFontColor().blend(state, elapsed);
	getParent()->drawText(getPositionFlag(), getText(), foreStyle, db->boundingBox, foreStyle.getShadowStyle().hasShadow, false);
}

void GMControlLabel::refresh()
{
	D(d);
	Base::refresh();
	d->foreStyle.refresh();
}

GMStyle& GMControlLabel::getStyle(Base::StyleType style)
{
	D(d);
	Base::StyleType s = (Base::StyleType)style;
	switch (s)
	{
	case GMControlLabel::ForeStyle:
		return d->foreStyle;
	default:
		return Base::getStyle(style);
	}
}

void GMControlLabel::initStyles(GMWidget* widget)
{
	D(d);
	d->foreStyle.setFont(0);
	d->foreStyle.setFontColor(GMControlState::Disabled, GMVec4(.87f, .87f, .87f, .87f));
}

const GMString& GMControlLabel::getText() const
{
	D(d);
	return d->text;
}

void GMControlLabel::setText(const GMString& text)
{
	D(d);
	d->text = text;
}

void GMControlLabel::setFontColor(const GMVec4& color)
{
	GMStyle& style = getStyle((GMControl::StyleType) ForeStyle);
	style.getFontColor().init(color);
}

class GMControlButtonBorder : public GMControlBorder
{
public:
	GMControlButtonBorder(GMWidget* widget) : GMControlBorder(widget) { }

private:
	virtual void render(GMDuration elapsed) override;
};

void GMControlButtonBorder::render(GMDuration elapsed)
{
	if (!getVisible())
		return;

	D(d);
	D_BASE(db, Base);
	GMWidget* widget = getParent();
	widget->drawBorder(getPositionFlag(), d->borderStyle, db->boundingBox, .8f);
}

GMControlButton* GMControlButton::createControl(
	GMWidget* widget,
	const GMString& text,
	GMint32 x,
	GMint32 y,
	GMint32 width,
	GMint32 height,
	bool isDefault
)
{
	GMControlButton* buttonControl = new GMControlButton(widget);
	buttonControl->setText(text);
	buttonControl->setPosition(x, y);
	buttonControl->setSize(width, height);
	buttonControl->setIsDefault(isDefault);
	return buttonControl;
}

GMControlButton::~GMControlButton()
{

}

GMControlButton::GMControlButton(GMWidget* widget) : Base(widget)
{
	GM_CREATE_DATA();
	D(d);
	d->fillBorder = gm_makeOwnedPtr<GMControlButtonBorder>(widget);
	d->foreBorder = gm_makeOwnedPtr<GMControlButtonBorder>(widget);
	initStyles(widget);
}

void GMControlButton::refresh()
{
	D(d);
	Base::refresh();
	d->fillStyle.refresh();
}

GMStyle& GMControlButton::getStyle(GMControl::StyleType style)
{
	D(d);
	D_BASE(db, Base);
	GMControl::StyleType s = (GMControl::StyleType)style;
	switch (s)
	{
	case GMControlButton::ForeStyle:
		return db->foreStyle;
	case GMControlButton::FillStyle:
		return d->fillStyle;
	default:
		return Base::getStyle(style);
	}
}

bool GMControlButton::onMouseDown(GMSystemMouseEvent* event)
{
	return handleMousePressOrDblClick(event->getPoint());
}

bool GMControlButton::onMouseDblClick(GMSystemMouseEvent* event)
{
	return handleMousePressOrDblClick(event->getPoint());
}

bool GMControlButton::onMouseUp(GMSystemMouseEvent* event)
{
	return handleMouseRelease(event->getPoint());
}

bool GMControlButton::canHaveFocus()
{
	return getEnabled() && getVisible();
}

bool GMControlButton::onKeyDown(GMSystemKeyEvent* event)
{
	D(d);
	if (event->getKey() == GMKey_Space)
	{
		d->pressed = true;
		return true;
	}
	return false;
}

bool GMControlButton::onKeyUp(GMSystemKeyEvent* event)
{
	D(d);
	if (event->getKey() == GMKey_Space)
	{
		if (d->pressed)
		{
			d->pressed = false;
			emitSignal(GM_SIGNAL(GMControlButton, click));
			return true;
		}
	}
	return false;
}

bool GMControlButton::containsPoint(GMPoint point)
{
	return GMControl::containsPoint(point);
}

void GMControlButton::render(GMDuration elapsed)
{
	if (!getVisible())
		return;

	D(d);
	D_BASE(db, Base);
	GMint32 offsetX = 0, offsetY = 0;
	GMControlState::State state = GMControlState::Normal;
	if (!getEnabled())
	{
		state = GMControlState::Disabled;
	}
	else if (d->pressed)
	{
		state = GMControlState::Pressed;
		offsetX = 1;
		offsetY = 2;
	}
	else if (getMouseOver())
	{
		state = GMControlState::MouseOver;
		offsetX = -1;
		offsetY = -2;
	}
	else if (hasFocus())
	{
		state = GMControlState::Focus;
	}

	GMRect rc = getBoundingRect();
	rc.x += offsetX;
	rc.width -= offsetX * 2;
	rc.y += offsetY;
	rc.height -= offsetY * 2;

	GMWidget* widget = getParent();
	GMfloat blendRate = (state == GMControlState::Pressed) ? 0.0f : 0.8f;

	GMStyle& foreStyle = getStyle((GMControl::StyleType)ForeStyle);
	foreStyle.getTextureColor().blend(state, elapsed, blendRate);
	foreStyle.getFontColor().blend(state, elapsed, blendRate);
	d->foreBorder->setPosition(rc.x, rc.y);
	d->foreBorder->setSize(rc.width, rc.height);
	d->foreBorder->setBorderStyle(foreStyle);
	d->foreBorder->render(elapsed);
	widget->drawText(getPositionFlag(), getText(), foreStyle, rc, foreStyle.getShadowStyle().hasShadow, true);

	GMStyle& fillStyle = getStyle((GMControl::StyleType)FillStyle);
	fillStyle.getTextureColor().blend(state, elapsed, blendRate);
	fillStyle.getFontColor().blend(state, elapsed, blendRate);
	d->fillBorder->setPosition(rc.x, rc.y);
	d->fillBorder->setSize(rc.width, rc.height);
	d->fillBorder->setBorderStyle(fillStyle);
	d->fillBorder->render(elapsed);
	widget->drawText(getPositionFlag(), getText(), fillStyle, rc, fillStyle.getShadowStyle().hasShadow, true);
}

bool GMControlButton::handleMousePressOrDblClick(const GMPoint& pt)
{
	if (!canHaveFocus())
		return false;

	D(d);
	D_BASE(db, GMControl);
	if (containsPoint(pt))
	{
		d->pressed = true;
		getParent()->getParentWindow()->setWindowCapture(true);

		if (!db->hasFocus)
			getParent()->requestFocus(this);
		return true;
	}

	return false;
}

bool GMControlButton::handleMouseRelease(const GMPoint& pt)
{
	if (!canHaveFocus())
		return false;

	D(d);
	D_BASE(db, GMControl);
	if (d->pressed)
	{
		d->pressed = false;
		getParent()->getParentWindow()->setWindowCapture(false);

		GMWidget* widget = getParent();
		if (!widget->canKeyboardInput())
			widget->clearFocus(widget);

		if (containsPoint(pt))
		{
			emitSignal(GM_SIGNAL(GMControlButton, click));
		}

		return true;
	}
	return false;
}

void GMControlButton::initStyles(GMWidget* widget)
{
	D(d);
	D_BASE(db, Base);

	db->foreStyle.setTexture(widget->getArea(GMTextureArea::ButtonArea));
	db->foreStyle.setFont(0);
	db->foreStyle.setFontColor(GMVec4(1, 1, 1, 1));
	db->foreStyle.setTextureColor(GMControlState::Normal, GMVec4(1.f, 1.f, 1.f, .58f));
	db->foreStyle.setTextureColor(GMControlState::Pressed, GMVec4(0, 0, 0, .78f));
	db->foreStyle.setFontColor(GMControlState::Normal, GMVec4(.3f, .3f, .3f, 1.f));
	db->foreStyle.setFontColor(GMControlState::MouseOver, GMVec4(0, 0, 0, 1.f));

	d->fillStyle.setTexture(widget->getArea(GMTextureArea::ButtonFillArea));
	d->fillStyle.setFont(0);
	d->fillStyle.setFontColor(GMVec4(1, 1, 1, 1));
	d->fillStyle.setTextureColor(GMControlState::MouseOver, GMVec4(1.f, 1.f, 1.f, .63f));
	d->fillStyle.setTextureColor(GMControlState::Pressed, GMVec4(1.f, 1.f, 1.f, .24f));
	d->fillStyle.setTextureColor(GMControlState::Focus, GMVec4(1.f, 1.f, 1.f, .18f));
	d->fillStyle.setFontColor(GMControlState::Normal, GMVec4(.3f, .3f, .3f, 1.f));
	d->fillStyle.setFontColor(GMControlState::MouseOver, GMVec4(0, 0, 0, 1.f));
}

GMControlBorder* GMControlBorder::createControl(
	GMWidget* widget,
	GMint32 x,
	GMint32 y,
	GMint32 width,
	GMint32 height,
	const GMRect& cornerRect
)
{
	GMControlBorder* borderControl = new GMControlBorder(widget);
	borderControl->setPosition(x, y);
	borderControl->setSize(width, height);
	return borderControl;
}

GM_DEFINE_PROPERTY(GMControlBorder, GMRect, Corner, corner);
GM_DEFINE_PROPERTY(GMControlBorder, GMStyle, BorderStyle, borderStyle);
GMControlBorder::GMControlBorder(GMWidget* widget) : Base(widget)
{
	GM_CREATE_DATA();
	initStyles(widget);
}

void GMControlBorder::render(GMDuration elapsed)
{
	if (!getVisible())
		return;

	D(d);
	D_BASE(db, Base);
	d->borderStyle.getTextureColor().blend(GMControlState::Normal, elapsed);
	GMWidget* widget = getParent();
	widget->drawBorder(getPositionFlag(), d->borderStyle, db->boundingBox, .8f);
}

bool GMControlBorder::containsPoint(GMPoint point)
{
	return false;
}

void GMControlBorder::initStyles(GMWidget* widget)
{
	D(d);
	d->borderStyle.setTexture(widget->getArea(GMTextureArea::BorderArea));
	d->borderStyle.setTextureColor(GMControlState::Normal, GMVec4(1.f, 1.f, 1.f, 1.f));
}

GM_PRIVATE_OBJECT_UNALIGNED(GMControlScrollBar)
{
	bool draggingThumb = false;
	bool showThumb = true;
	bool canRequestFocus = true;
	GMint32 maximum = 10;
	GMint32 minimum = 0;
	GMint32 pageStep = 1;
	GMint32 singleStep = 1;
	GMint32 value = 0;

	GMPoint mousePt; //!< 相对于GMControlScrollBar本身的坐标
	GMRect rcUp;
	GMRect rcDown;
	GMRect rcThumb;
	GMRect rcTrack;
	GMfloat thumbOffset = 0;
	GMControlScrollBarArrowState arrowState = GMControlScrollBarArrowState::Clear;
	GMfloat arrowTime = 0;

	GMStyle styleUp;
	GMStyle styleDown;
	GMOwnedPtr<GMControlBorder> thumb;
	GMStyle styleTrack;

	GMfloat allowClickDelay = .33f;
	GMfloat allowClickRepeat = .05f;
};

bool GMControlScrollBar::handleMouse(GMSystemMouseEvent* event)
{
	D(d);
	d->mousePt = event->getPoint();
	return Base::handleMouse(event);
}

void GMControlScrollBar::render(GMDuration elapsed)
{
	D(d);
	if (d->arrowState != GMControlScrollBarArrowState::Clear)
	{
		GMfloat now = GM.getRunningStates().elapsedTime;
		if (GM_inRect(d->rcUp, d->mousePt))
		{
			switch (d->arrowState)
			{
			case GMControlScrollBarArrowState::ClickedUp:
			{
				if (d->allowClickDelay < now - d->arrowTime)
				{
					scroll(-d->singleStep);
					d->arrowState = GMControlScrollBarArrowState::HeldUp;
					d->arrowTime = now;
				}
				break;
			}
			case GMControlScrollBarArrowState::HeldUp:
			{
				if (d->allowClickRepeat < now - d->arrowTime)
				{
					scroll(-d->singleStep);
					d->arrowTime = now;
				}
				break;
			}
			}
		}
		else if (GM_inRect(d->rcDown, d->mousePt))
		{
			switch (d->arrowState)
			{
			case GMControlScrollBarArrowState::ClickedDown:
			{
				if (d->allowClickDelay < now - d->arrowTime)
				{
					scroll(d->singleStep);
					d->arrowState = GMControlScrollBarArrowState::HeldDown;
					d->arrowTime = now;
				}
				break;
			}
			case GMControlScrollBarArrowState::HeldDown:
			{
				if (d->allowClickRepeat < now - d->arrowTime)
				{
					scroll(d->singleStep);
					d->arrowTime = now;
				}
				break;
			}
			}
		}
		else if (GM_inRect(d->rcTrack, d->mousePt) && !(GM_inRect(d->rcThumb, d->mousePt)))
		{
			if (d->mousePt.y < d->rcThumb.y)
			{
				switch (d->arrowState)
				{
				case GMControlScrollBarArrowState::TrackUp:
				{
					if (d->allowClickDelay < now - d->arrowTime)
					{
						scroll(-d->pageStep);
						d->arrowState = GMControlScrollBarArrowState::TrackHeldUp;
						d->arrowTime = now;
					}
					break;
				}
				case GMControlScrollBarArrowState::TrackHeldUp:
				{
					if (d->allowClickRepeat < now - d->arrowTime)
					{
						scroll(-d->pageStep);
						d->arrowTime = now;
					}
					break;
				}
				}
			}
			else if (d->mousePt.y > d->rcThumb.y + d->rcThumb.height)
			{
				switch (d->arrowState)
				{
				case GMControlScrollBarArrowState::TrackDown:
				{
					if (d->allowClickDelay < now - d->arrowTime)
					{
						scroll(d->pageStep);
						d->arrowState = GMControlScrollBarArrowState::TrackHeldDown;
						d->arrowTime = now;
					}
					break;
				}
				case GMControlScrollBarArrowState::TrackHeldDown:
				{
					if (d->allowClickRepeat < now - d->arrowTime)
					{
						scroll(d->pageStep);
						d->arrowTime = now;
					}
					break;
				}
			}
			}
		}
	}

	GMControlState::State state = GMControlState::Normal;
	if (!getEnabled() || !d->showThumb)
	{
		state = GMControlState::Disabled;
	}
	else if (getMouseOver())
	{
		state = GMControlState::MouseOver;
	}
	else if (hasFocus())
	{
		state = GMControlState::Focus;
	}
	else if (!getVisible())
	{
		state = GMControlState::Hidden;
	}

	d->styleUp.getTextureColor().blend(state, elapsed);
	d->styleDown.getTextureColor().blend(state, elapsed);
	d->styleTrack.getTextureColor().blend(state, elapsed);

	GMWidget* widget = getParent();
	GM_ASSERT(widget);
	widget->drawSprite(getPositionFlag(), d->styleUp, d->rcUp, .99f);
	widget->drawSprite(getPositionFlag(), d->styleDown, d->rcDown, .99f);
	widget->drawSprite(getPositionFlag(), d->styleTrack, d->rcTrack, .99f);
	if (d->showThumb)
	{
		const GMRect& thumbCorner = d->thumb->getBorderStyle().getCornerRc();
		if (thumbCorner.width * 2 < d->rcThumb.width && thumbCorner.height * 2 < d->rcThumb.height)
		{
			d->thumb->setPositionFlag(getPositionFlag());
			d->thumb->setPosition(d->rcThumb.x, d->rcThumb.y);
			d->thumb->setSize(d->rcThumb.width, d->rcThumb.height);
			d->thumb->render(elapsed);
		}
		else
		{
			// 绘制空间不足，使用整个素材来绘制
			GMStyle& thumbStyle = getStyle((GMControl::StyleType) Thumb);
			thumbStyle.getTextureColor().blend(GMControlState::Normal, elapsed);
			widget->drawSprite(getPositionFlag(), thumbStyle, d->rcThumb, .99f);
		}
	}
}

void GMControlScrollBar::initStyles(GMWidget* widget)
{
	D(d);
	GMStyle styleTemplate;
	styleTemplate.setTextureColor(GMControlState::Normal, GMVec4(1, 1, 1, .58f));
	styleTemplate.setTextureColor(GMControlState::Focus, GMVec4(1, 1, 1, .78f));
	styleTemplate.setTextureColor(GMControlState::Disabled, GMVec4(1, 1, 1, .27f));

	GMStyle& styleUp = d->styleUp;
	styleUp = styleTemplate;
	styleUp.setTexture(widget->getArea(GMTextureArea::ScrollBarUp));
	styleUp.setTextureColor(GMControlState::Disabled, GMVec4(.87f, .87f, .87f, 1));

	GMStyle& styleDown = d->styleDown;
	styleDown = styleTemplate;
	styleDown.setTexture(widget->getArea(GMTextureArea::ScrollBarDown));
	styleDown.setTextureColor(GMControlState::Disabled, GMVec4(.87f, .87f, .87f, 1));

	GMStyle& styleTrack = d->styleTrack;
	styleTrack = styleTemplate;
	styleTrack.setTexture(widget->getArea(GMTextureArea::ScrollBarTrack));
	styleTrack.setTextureColor(GMControlState::Disabled, GMVec4(.87f, .87f, .87f, 1));
}

class GMControlScrollBarThumb : public GMControlBorder
{
public:
	enum StyleType
	{
		Thumb
	};

public:
	GMControlScrollBarThumb(GMWidget* widget) : GMControlBorder(widget) { initStyles(widget); }

public:
	virtual GMStyle& getStyle(GMControl::StyleType style) override;

private:
	void initStyles(GMWidget* widget);
};

void GMControlScrollBarThumb::initStyles(GMWidget* widget)
{
	D(d);
	d->borderStyle.setTextureColor(GMControlState::Normal, GMVec4(1, 1, 1, .58f));
	d->borderStyle.setTextureColor(GMControlState::Focus, GMVec4(1, 1, 1, .78f));
	d->borderStyle.setTextureColor(GMControlState::Disabled, GMVec4(1, 1, 1, .27f));
	d->borderStyle.setTexture(widget->getArea(GMTextureArea::ScrollBarThumb));
}

GMStyle& GMControlScrollBarThumb::getStyle(GMControl::StyleType style)
{
	D(d);
	if (style == (GMControl::StyleType)Thumb)
		return d->borderStyle;
	return GMControl::getStyle(style);
}

GMControlScrollBar* GMControlScrollBar::createControl(
	GMWidget* widget,
	GMint32 x,
	GMint32 y,
	GMint32 width,
	GMint32 height,
	bool isDefault
)
{
	GMControlScrollBar* scrollBar = new GMControlScrollBar(widget);
	scrollBar->setPosition(x, y);
	scrollBar->setSize(width, height);
	scrollBar->setIsDefault(isDefault);
	return scrollBar;
}

GM_DEFINE_PROPERTY(GMControlScrollBar, GMint32, PageStep, pageStep)
GM_DEFINE_PROPERTY(GMControlScrollBar, GMint32, SingleStep, singleStep)
GM_DEFINE_GETTER(GMControlScrollBar, GMint32, Value, value)
GM_DEFINE_GETTER(GMControlScrollBar, GMint32, Maximum, maximum)
GM_DEFINE_GETTER(GMControlScrollBar, GMint32, Minimum, minimum)
GM_DEFINE_PROPERTY(GMControlScrollBar, bool, CanRequestFocus, canRequestFocus)
GMControlScrollBar::GMControlScrollBar(GMWidget* widget)
	: Base(widget) 
{
	GM_CREATE_DATA();
	D(d);
	initStyles(widget);
	d->thumb = gm_makeOwnedPtr<GMControlScrollBarThumb>(widget);
}

GMControlScrollBar::~GMControlScrollBar()
{

}

void GMControlScrollBar::setMaximum(GMint32 maximum)
{
	D(d);
	if (d->maximum != maximum)
	{
		d->maximum = maximum;
		updateThumbRect();
	}
}

void GMControlScrollBar::setMinimum(GMint32 minimum)
{
	D(d);
	if (d->minimum != minimum)
	{
		d->minimum = minimum;
		updateThumbRect();
	}
}

void GMControlScrollBar::setValue(GMint32 value)
{
	D(d);
	if (d->value != value)
	{
		d->value = value;
	}

	updateThumbRect();
}

void GMControlScrollBar::updateRect()
{
	Base::updateRect();

	// 调整部件大小
	D(d);
	D_BASE(db, Base);
	d->rcUp = db->boundingBox;
	d->rcUp.height = db->boundingBox.width;

	d->rcDown = db->boundingBox;
	d->rcDown.height = db->boundingBox.width;
	d->rcDown.y = db->boundingBox.y + db->boundingBox.height - db->boundingBox.width;

	d->rcTrack = d->rcUp;
	d->rcTrack.y = d->rcUp.y + d->rcUp.height;
	d->rcTrack.height = d->rcDown.y - d->rcTrack.y;

	d->rcThumb.x = db->boundingBox.x;
	d->rcThumb.width = db->boundingBox.width;

	updateThumbRect();
}

bool GMControlScrollBar::canHaveFocus()
{
	return getCanRequestFocus();
}

GMStyle& GMControlScrollBar::getStyle(GMControl::StyleType style)
{
	D(d);
	D_BASE(db, Base);
	StyleType s = (StyleType)style;
	switch (s)
	{
	case GMControlScrollBar::ArrowUp:
		return d->styleUp;
	case GMControlScrollBar::ArrowDown:
		return d->styleDown;
	case GMControlScrollBar::Track:
		return d->styleTrack;
	case GMControlScrollBar::Thumb:
		return d->thumb->getStyle((GMControl::StyleType) GMControlScrollBarThumb::Thumb);
	default:
		GM_ASSERT(false);
		return Base::getStyle(style);
	}
}

bool GMControlScrollBar::onMouseDown(GMSystemMouseEvent* event)
{
	return handleMouseClick(event);
}

bool GMControlScrollBar::onMouseDblClick(GMSystemMouseEvent* event)
{
	return handleMouseClick(event);
}

bool GMControlScrollBar::onMouseUp(GMSystemMouseEvent* event)
{
	D(d);
	GMWidget* widget = getParent();
	GM_ASSERT(widget);
	IWindow* window = widget->getParentWindow();
	GM_ASSERT(window);
	window->setWindowCapture(false);
	d->draggingThumb = false;
	d->arrowState = GMControlScrollBarArrowState::Clear;
	updateThumbRect();
	emitSignal(GM_SIGNAL(GMControlScrollBar, endDragThumb));
	return false;
}

bool GMControlScrollBar::onMouseMove(GMSystemMouseEvent* event)
{
	D(d);
	if (d->draggingThumb)
	{
		d->rcThumb.y = d->mousePt.y - d->thumbOffset;
		GMint32 maximum = d->rcTrack.y + d->rcTrack.height - d->rcThumb.height;
		if (d->rcThumb.y > maximum)
			d->rcThumb.y = maximum;

		GMint32 minimum = d->rcTrack.y;
		if (d->rcThumb.y < minimum)
			d->rcThumb.y = minimum;

		// 计算出一个最接近的值
		GMint32 value = Round(static_cast<GMfloat>(d->rcThumb.y - minimum) * getMaximum() / (maximum - minimum)) + getMinimum();
		if (value > getMaximum())
			value = getMaximum();
		setValue(value);
		emitSignal(GM_SIGNAL(GMControlScrollBar, valueChanged));

		return true;
	}
	return false;
}

bool GMControlScrollBar::onCaptureChanged(GMSystemCaptureChangedEvent* event)
{
	D(d);
	if (getParent()->getParentWindow()->getWindowHandle() != event->getCapturedWindow())
	{
		d->draggingThumb = false;
		emitSignal(GM_SIGNAL(GMControlScrollBar, endDragThumb));
	}
	return false;
}

void GMControlScrollBar::updateThumbRect()
{
	D(d);
	if (getMinimum() < getMaximum() &&
		getMinimum() <= getValue() && getValue() <= getMaximum())
	{
		d->showThumb = true;
		GMint32 pageSize = getMaximum() - getMinimum() + getPageStep();
		if (pageSize <= 0)
			pageSize = 1;
		d->rcThumb.height = getPageStep() * d->rcTrack.height / pageSize;
		d->rcThumb.y = d->rcTrack.y + (getValue() - getMinimum()) * (d->rcTrack.height - d->rcThumb.height) / (getMaximum() - getMinimum());
	}
	else
	{
		d->showThumb = false;
	}
}

bool GMControlScrollBar::handleMouseClick(GMSystemMouseEvent* event)
{
	D(d);
	GMWidget* widget = getParent();
	GM_ASSERT(widget);
	IWindow* window = widget->getParentWindow();
	GM_ASSERT(window);

	if (!hasFocus())
		widget->requestFocus(this);

	GMfloat nowElapsed = GM.getRunningStates().elapsedTime;
	if (GM_inRect(d->rcUp, d->mousePt))
	{
		if (window)
			window->setWindowCapture(true);

		d->value -= d->singleStep;
		if (d->value < d->minimum)
			d->value = d->minimum;
		updateThumbRect();
		emitSignal(GM_SIGNAL(GMControlScrollBar, valueChanged));
		d->arrowState = GMControlScrollBarArrowState::ClickedUp;
		d->arrowTime = nowElapsed;
		return true;
	}
	
	if (GM_inRect(d->rcDown, d->mousePt))
	{
		if (window)
			window->setWindowCapture(true);

		d->value += d->singleStep;
		if (d->value > d->maximum)
			d->value = d->maximum;
		updateThumbRect();
		emitSignal(GM_SIGNAL(GMControlScrollBar, valueChanged));
		d->arrowState = GMControlScrollBarArrowState::ClickedDown;
		d->arrowTime = nowElapsed;
		return true;
	}

	if (GM_inRect(d->rcThumb, d->mousePt))
	{
		if (window)
			window->setWindowCapture(true);

		d->thumbOffset = d->mousePt.y - d->rcThumb.y;
		d->draggingThumb = true;
		emitSignal(GM_SIGNAL(GMControlScrollBar, startDragThumb));
		return true;
	}

	if (GM_inRect(d->rcTrack, d->mousePt))
	{
		if (window)
			window->setWindowCapture(true);

		// 区分一下是点在了滑块上侧还是下侧
		if (d->mousePt.y < d->rcThumb.y)
		{
			scroll(-d->pageStep);
			d->arrowState = GMControlScrollBarArrowState::TrackUp;
			d->arrowTime = nowElapsed;
			return true;
		}
		else if (d->mousePt.y > d->rcThumb.y + d->rcThumb.height)
		{
			scroll(d->pageStep);
			d->arrowState = GMControlScrollBarArrowState::TrackDown;
			d->arrowTime = nowElapsed;
			return true;
		}
	}

	return false;
}

void GMControlScrollBar::clampValue()
{
	D(d);
	if (d->value > getMaximum())
		setValue(getMaximum());

	if (d->value < getMinimum())
		setValue(getMinimum());
}

void GMControlScrollBar::scroll(GMint32 value)
{
	D(d);
	d->value += value;
	clampValue();
	updateThumbRect();
	emitSignal(GM_SIGNAL(GMControlScrollBar, valueChanged));
}

END_NS