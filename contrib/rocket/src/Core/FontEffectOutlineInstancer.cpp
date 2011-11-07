/*
 * This source file is part of libRocket, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://www.librocket.com
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "precompiled.h"
#include "FontEffectOutlineInstancer.h"
#include "FontEffectOutline.h"

namespace Rocket {
namespace Core {

FontEffectOutlineInstancer::FontEffectOutlineInstancer()
{
	RegisterProperty("width", "1", true)
		.AddParser("number");
}

FontEffectOutlineInstancer::~FontEffectOutlineInstancer()
{
}

// Instances an outline font effect.
FontEffect* FontEffectOutlineInstancer::InstanceFontEffect(const String& ROCKET_UNUSED(name), const PropertyDictionary& properties)
{
	float width = properties.GetProperty("width")->Get< float >();

	FontEffectOutline* font_effect = new FontEffectOutline();
	if (font_effect->Initialise(Math::RealToInteger(width)))
		return font_effect;

	font_effect->RemoveReference();
	ReleaseFontEffect(font_effect);
	return NULL;
}

// Releases the given font effect.
void FontEffectOutlineInstancer::ReleaseFontEffect(FontEffect* font_effect)
{
	delete font_effect;
}

// Releases the instancer.
void FontEffectOutlineInstancer::Release()
{
	delete this;
}

}
}
