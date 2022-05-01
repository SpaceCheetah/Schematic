#include "Resources.h"
#include "Item.h"
#include <wx/graphics.h>

static_assert(sizeof(int) == sizeof(size_t) / 2); //Required for hashing

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
    wxBitmap initBitmap(int size) {
        wxImage image{size, size};
        image.InitAlpha();
        memset(image.GetAlpha(), 0, size * size); //set to fully transparent
        return {image};
    }
    struct HashPairIntBool {
        size_t operator() (std::pair<int,bool> key) const {
            return key.first | (static_cast<size_t>(key.second) << (sizeof(int) * 8 + 1));
        }
    };
    struct HashPairIntInt {
        size_t operator() (std::pair<int,int> key) const {
            return (static_cast<size_t>(key.first) << (sizeof(int) * 8)) | key.second;
        }
    };
    double rScale(double d, double scale) { //radial scale (scaling point from center)
        return 0.5 + (d - 0.5) * scale;
    }
}

wxBitmap resources::getBinBitmap(int size) { //doing it this way instead of image.Scale to get antialiasing
    static wxImage binImage{"res/bin.png", wxBITMAP_TYPE_PNG};
    wxBitmap binFullBitmap{binImage};
    wxBitmap bitmap{initBitmap(size)};
    wxMemoryDC dc{bitmap};
    wxGraphicsContext* context = wxGraphicsContext::Create(dc);
    context->DrawBitmap(binFullBitmap, 0, 0, size, size);
    delete context;
    dc.SelectObject(wxNullBitmap);
    return bitmap;
}

wxBitmap resources::getResistorBitmap(int size, bool rotated) {
    static std::unordered_map<std::pair<int,bool>,wxBitmap,HashPairIntBool> cache{};
    std::pair<int,bool> key{size, rotated};
    auto iter = cache.find(key);
    if(iter != cache.end()) {
        return iter->second;
    }
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
    cache[key] = bitmap;
    return bitmap;
}

wxIconBundle resources::getResistorIconBundle() {
    return wxIconBundle("res/resistor-multires.ico", wxBITMAP_TYPE_ICO);
}

//Only called once, so no need to cache
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

wxBitmap resources::getVoltSourceBitmap(int size, int shape, bool toolbar) {
    static std::unordered_map<std::pair<int, int>, wxBitmap, HashPairIntInt> cache{};
    std::pair<int, int> key{size, shape};
    if(!toolbar) {
        auto iter = cache.find(key);
        if (iter != cache.end()) {
            return iter->second;
        }
    }
    double scale = toolbar ? 1 : 0.4 / 0.7;
    wxBitmap bitmap{initBitmap(size)};
    wxMemoryDC dc{bitmap};
    wxGraphicsContext* context = wxGraphicsContext::Create(dc);
    wxPen pen = wxPen{wxPenInfo(*wxBLACK, std::ceil(size * 22.0 / 1024))};
    context->SetPen(pen);
    if(shape & Item::DEPENDENT) { //Draw border
        const wxPoint2DDouble points[] = {
                {0.5 * size, rScale(0.15, scale) * size},
                {rScale(0.85, scale) * size, 0.5 * size},
                {0.5 * size, rScale(0.85, scale) * size},
                {rScale(0.15, scale) * size, 0.5 * size},
                {0.5 * size, rScale(0.15, scale) * size}};
        context->StrokeLines(5, points);
    }
    else {
        context->DrawEllipse(rScale(0.15, scale) * size, rScale(0.15, scale) * size, 0.7 * scale * size, 0.7 * scale * size);
    }
    if((shape & Item::UP) || (shape & Item::DOWN)) { //Draw wire connections
        context->StrokeLine(0.5 * size, 0, 0.5 * size, rScale(0.15, scale) * size);
        context->StrokeLine(0.5 * size, rScale(0.85, scale) * size, 0.5 * size, size);
    } else {
        context->StrokeLine(0, 0.5 * size, rScale(0.15, scale) * size, 0.5 * size);
        context->StrokeLine(rScale(0.85, scale) * size, 0.5 * size, size, 0.5 * size);
    }
    if(shape & Item::UP) { //Draw + and -
        context->StrokeLine(0.5 * size, rScale(0.3, scale) * size, 0.5 * size, 0.5 * size);
        context->StrokeLine(rScale(0.4, scale) * size, rScale(0.4, scale) * size, rScale(0.6, scale) * size, rScale(0.4, scale) * size);
        context->StrokeLine(rScale(0.4, scale) * size, rScale(0.67, scale) * size, rScale(0.6, scale) * size, rScale(0.67, scale) * size);
    } else if(shape & Item::RIGHT) {
        context->StrokeLine(rScale(0.7, scale) * size, 0.5 * size, 0.5 * size, 0.5 * size);
        context->StrokeLine(rScale(0.6, scale) * size, rScale(0.4, scale) * size, rScale(0.6, scale) * size, rScale(0.6, scale) * size);
        context->StrokeLine(rScale(0.33, scale) * size, rScale(0.4, scale) * size, rScale(0.33, scale) * size, rScale(0.6, scale) * size);
    } else if(shape & Item::DOWN) {
        context->StrokeLine(0.5 * size, rScale(0.7, scale) * size, 0.5 * size, 0.5 * size);
        context->StrokeLine(rScale(0.4, scale) * size, rScale(0.6, scale) * size, rScale(0.6, scale) * size, rScale(0.6, scale) * size);
        context->StrokeLine(rScale(0.4, scale) * size, rScale(0.32, scale) * size, rScale(0.6, scale) * size, rScale(0.32, scale) * size);
    } else {
        context->StrokeLine(rScale(0.3, scale) * size, 0.5 * size, 0.5 * size, 0.5 * size);
        context->StrokeLine(rScale(0.4, scale) * size, rScale(0.4, scale) * size, rScale(0.4, scale) * size, rScale(0.6, scale) * size);
        context->StrokeLine(rScale(0.67, scale) * size, rScale(0.4, scale) * size, rScale(0.67, scale) * size, rScale(0.6, scale) * size);
    }
    delete context;
    dc.SelectObject(wxNullBitmap);
    if(!toolbar) cache[key] = bitmap;
    return bitmap;
}

wxBitmap resources::getAmpSourceBitmap(int size, int shape, bool toolbar) {
    static std::unordered_map<std::pair<int,int>,wxBitmap,HashPairIntInt> cache{};
    std::pair<int,int> key{size, shape};
    if(!toolbar) {
        auto iter = cache.find(key);
        if (iter != cache.end()) {
            return iter->second;
        }
    }
    wxBitmap bitmap{initBitmap(size)};
    wxMemoryDC dc{bitmap};
    wxGraphicsContext* context = wxGraphicsContext::Create(dc);
    wxPen pen = wxPen{wxPenInfo(*wxBLACK, std::ceil(size * 22.0 / 1024))};
    context->SetPen(pen);
    double scale = toolbar ? 1 : 0.4 / 0.7;
    if(shape & Item::DEPENDENT) { //Draw border
        const wxPoint2DDouble points[] = {
                {0.5 * size, rScale(0.15, scale) * size},
                {rScale(0.85, scale) * size, 0.5 * size},
                {0.5 * size, rScale(0.85, scale) * size},
                {rScale(0.15, scale) * size, 0.5 * size},
                {0.5 * size, rScale(0.15, scale) * size}};
        context->StrokeLines(5, points);
    }
    else {
        context->DrawEllipse(rScale(0.15, scale) * size, rScale(0.15, scale) * size, 0.7 * scale * size, 0.7 * scale * size);
    }
    if((shape & Item::UP) || (shape & Item::DOWN)) { //Draw wire connections and main part of arrow
        context->StrokeLine(0.5 * size, 0, 0.5 * size, rScale(0.15, scale) * size);
        context->StrokeLine(0.5 * size, rScale(0.85, scale) * size, 0.5 * size, size);
        context->StrokeLine(0.5 * size, rScale(0.7, scale) * size, 0.5 * size, rScale(0.3, scale) * size);
    } else {
        context->StrokeLine(0, 0.5 * size, rScale(0.15, scale) * size, 0.5 * size);
        context->StrokeLine(rScale(0.85, scale) * size, 0.5 * size, size, 0.5 * size);
        context->StrokeLine(rScale(0.3, scale) * size, 0.5 * size, rScale(0.7, scale) * size, 0.5 * size);
    }
    if(shape & Item::UP) { //Draw current arrow
        const wxPoint2DDouble points[] = {{rScale(0.47, scale) * size, rScale(0.4, scale) * size},
                                          {0.5 * size, rScale(0.3, scale) * size},
                                          {rScale(0.53, scale) * size, rScale(0.4, scale) * size}};
        context->StrokeLines(3, points);
    } else if(shape & Item::RIGHT) {
        const wxPoint2DDouble points[] = {{rScale(0.6, scale) * size, rScale(0.47, scale) * size},
                                          {rScale(0.7, scale) * size, 0.5 * size},
                                          {rScale(0.6, scale) * size, rScale(0.53, scale) * size}};
        context->StrokeLines(3, points);
    } else if(shape & Item::DOWN) {
        const wxPoint2DDouble points[] = {{rScale(0.47, scale) * size, rScale(0.6, scale) * size},
                                          {0.5 * size, rScale(0.7, scale) * size},
                                          {rScale(0.53, scale) * size, rScale(0.6, scale) * size}};
        context->StrokeLines(3, points);
    } else {
        const wxPoint2DDouble points[] = {{rScale(0.4, scale) * size, rScale(0.47, scale) * size},
                                          {rScale(0.3, scale) * size, 0.5 * size},
                                          {rScale(0.4, scale) * size, rScale(0.53, scale) * size}};
        context->StrokeLines(3, points);
    }
    delete context;
    dc.SelectObject(wxNullBitmap);
    if(!toolbar) cache[key] = bitmap;
    return bitmap;
}

wxBitmap resources::getCapacitorBitmap(int size, bool rotated) {
    static std::unordered_map<std::pair<int,bool>,wxBitmap,HashPairIntBool> cache{};
    std::pair<int,bool> key{size, rotated};
    auto iter = cache.find(key);
    if(iter != cache.end()) {
        return iter->second;
    }
    wxBitmap bitmap{initBitmap(size)};
    wxMemoryDC dc{bitmap};
    wxGraphicsContext* context = wxGraphicsContext::Create(dc);
    wxPen pen = wxPen{wxPenInfo(*wxBLACK, std::ceil(size * 22.0 / 1024))};
    context->SetPen(pen);
    if(rotated) {
        context->StrokeLine(0.5 * size, 0, 0.5 * size, 0.42 * size);
        context->StrokeLine(0.5 * size, 0.58 * size, 0.5 * size, size);
        context->StrokeLine(0.3 * size, 0.42 * size, 0.7 * size, 0.42 * size);
        context->StrokeLine(0.3 * size, 0.58 * size, 0.7 * size, 0.58 * size);
    } else {
        context->StrokeLine(0, 0.5 * size, 0.42 * size, 0.5 * size);
        context->StrokeLine(0.58 * size, 0.5 * size, size, 0.5 * size);
        context->StrokeLine(0.42 * size, 0.3 * size, 0.42 * size, 0.7 * size);
        context->StrokeLine(0.58 * size, 0.3 * size, 0.58 * size, 0.7 * size);
    }
    delete context;
    dc.SelectObject(wxNullBitmap);
    cache[key] = bitmap;
    return bitmap;
}
