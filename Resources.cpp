#include "Resources.h"
#include <wx/graphics.h>

wxImage resources::getBinImage() {
    static wxImage binImage{"res/bin.png", wxBITMAP_TYPE_PNG};
    return binImage;
}

namespace {
    const wxPoint2DDouble resistorPoints[] {
            {0, 0.5},
            {0.140625, 0.5},
            {0.1875, 0.59375},
            {0.28125, 0.40625},
            {0.375, 0.59375},
            {0.453125, 0.40625},
            {0.53125, 0.59375},
            {0.625, 0.40625},
            {0.703125, 0.59375},
            {0.796875, 0.40625},
            {0.84375, 0.5},
            {1, 0.5}
    };
}

namespace {
    wxBitmap initBitmap(int size) {
        wxImage image{size, size};
        image.InitAlpha();
        memset(image.GetAlpha(), 0, size * size); //set to fully transparent
        return {image};
    }
}

wxBitmap resources::getResistorBitmap(int size, bool rotated) {
    wxBitmap bitmap{initBitmap(size)};
    wxMemoryDC dc{bitmap};
    wxGraphicsContext* context = wxGraphicsContext::Create(dc);
    wxPen pen = wxPen{wxPenInfo(*wxBLACK, std::ceil(size * 22.0 / 1024))};
    context->SetPen(pen);
    constexpr size_t numPoints = sizeof(resistorPoints) / sizeof(resistorPoints[0]);
    wxPoint2DDouble points[numPoints];
    for(int i = 0; i < numPoints; i ++) {
        if(rotated) {
            points[i] = {resistorPoints[i].m_y * size, resistorPoints[i].m_x * size};
        } else {
            points[i] = resistorPoints[i] * size;
        }
    }
    context->StrokeLines(numPoints, points);
    delete context;
    dc.SelectObject(wxNullBitmap);
    return bitmap;
}

wxBitmap resources::getWireBitmap(int size) {
    wxBitmap bitmap{initBitmap(size)};
    wxMemoryDC dc{bitmap};
    wxGraphicsContext* context = wxGraphicsContext::Create(dc);
    wxPen pen = wxPen{wxPenInfo(*wxBLACK, std::ceil(size * 22.0 / 1024))};
    context->SetPen(pen);
    context->StrokeLine(0, size / 2.0, size, size / 2.0);
    delete context;
    dc.SelectObject(wxNullBitmap);
    return bitmap;
}
