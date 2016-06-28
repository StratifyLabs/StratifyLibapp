//Copyright 2011-2016 Tyler Gilbert; All Rights Reserved

#include <ui/TabText.hpp>
#include "draw.hpp"
using namespace ui;

TabText::TabText(){
	text().set_align_bottom();
}

void TabText::draw(const DrawingAttr & attr){
	text().draw(attr);
}
