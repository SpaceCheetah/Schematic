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

wxBitmap resources::getVoltSourceBitmap(int size, int shape) {
    static std::unordered_map<std::pair<int,int>,wxBitmap,HashPairIntInt> cache{};
    std::pair<int,int> key{size, shape};
    auto iter = cache.find(key);
    if(iter != cache.end()) {
        return iter->second;
    }
    wxBitmap bitmap{initBitmap(size)};
    wxMemoryDC dc{bitmap};
    wxGraphicsContext* context = wxGraphicsContext::Create(dc);
    wxPen pen = wxPen{wxPenInfo(*wxBLACK, std::ceil(size * 22.0 / 1024))};
    context->SetPen(pen);
    if(shape & Item::DEPENDENT) { //Draw border
        const wxPoint2DDouble points[] = {
                {0.5 * size, 0.15 * size},
                {0.85 * size, 0.5 * size},
                {0.5 * size, 0.85 * size},
                {0.15 * size, 0.5 * size},
                {0.5 * size, 0.15 * size}};
        context->StrokeLines(5, points);
    }
    else {
        context->DrawEllipse(0.15 * size, 0.15 * size, 0.7 * size, 0.7 * size);
    }
    if((shape & Item::UP) || (shape & Item::DOWN)) { //Draw wire connections
        context->StrokeLine(0.5 * size, 0, 0.5 * size, 0.15 * size);
        context->StrokeLine(0.5 * size, 0.85 * size, 0.5 * size, size);
    } else {
        context->StrokeLine(0, 0.5 * size, 0.15 * size, 0.5 * size);
        context->StrokeLine(0.85 * size, 0.5 * size, size, 0.5 * size);
    }
    if(shape & Item::UP) { //Draw + and -
        context->StrokeLine(0.5 * size, 0.3 * size, 0.5 * size, 0.5 * size);
        context->StrokeLine(0.4 * size, 0.4 * size, 0.6 * size, 0.4 * size);
        context->StrokeLine(0.4 * size, 0.7 * size, 0.6 * size, 0.7 * size);
    } else if(shape & Item::RIGHT) {
        context->StrokeLine(0.7 * size, 0.5 * size, 0.5 * size, 0.5 * size);
        context->StrokeLine(0.6 * size, 0.4 * size, 0.6 * size, 0.6 * size);
        context->StrokeLine(0.3 * size, 0.4 * size, 0.3 * size, 0.6 * size);
    } else if(shape & Item::DOWN) {
        context->StrokeLine(0.5 * size, 0.7 * size, 0.5 * size, 0.5 * size);
        context->StrokeLine(0.4 * size, 0.6 * size, 0.6 * size, 0.6 * size);
        context->StrokeLine(0.4 * size, 0.3 * size, 0.6 * size, 0.3 * size);
    } else {
        context->StrokeLine(0.3 * size, 0.5 * size, 0.5 * size, 0.5 * size);
        context->StrokeLine(0.4 * size, 0.4 * size, 0.4 * size, 0.6 * size);
        context->StrokeLine(0.7 * size, 0.4 * size, 0.7 * size, 0.6 * size);
    }
    delete context;
    dc.SelectObject(wxNullBitmap);
    cache[key] = bitmap;
    return bitmap;
}

wxBitmap resources::getAmpSourceBitmap(int size, int shape) {
    static std::unordered_map<std::pair<int,int>,wxBitmap,HashPairIntInt> cache{};
    std::pair<int,int> key{size, shape};
    auto iter = cache.find(key);
    if(iter != cache.end()) {
        return iter->second;
    }
    wxBitmap bitmap{initBitmap(size)};
    wxMemoryDC dc{bitmap};
    wxGraphicsContext* context = wxGraphicsContext::Create(dc);
    wxPen pen = wxPen{wxPenInfo(*wxBLACK, std::ceil(size * 22.0 / 1024))};
    context->SetPen(pen);
    if(shape & Item::DEPENDENT) { //Draw border
        const wxPoint2DDouble points[] = {
                {0.5 * size, 0.15 * size},
                {0.85 * size, 0.5 * size},
                {0.5 * size, 0.85 * size},
                {0.15 * size, 0.5 * size},
                {0.5 * size, 0.15 * size}};
        context->StrokeLines(5, points);
    }
    else {
        context->DrawEllipse(0.15 * size, 0.15 * size, 0.7 * size, 0.7 * size);
    }
    if((shape & Item::UP) || (shape & Item::DOWN)) { //Draw wire connections and main part of arrow
        context->StrokeLine(0.5 * size, 0, 0.5 * size, 0.15 * size);
        context->StrokeLine(0.5 * size, 0.85 * size, 0.5 * size, size);
        context->StrokeLine(0.5 * size, 0.7 * size, 0.5 * size, 0.3 * size);
    } else {
        context->StrokeLine(0, 0.5 * size, 0.15 * size, 0.5 * size);
        context->StrokeLine(0.85 * size, 0.5 * size, size, 0.5 * size);
        context->StrokeLine(0.3 * size, 0.5 * size, 0.7 * size, 0.5 * size);
    }
    if(shape & Item::UP) { //Draw current arrow
        const wxPoint2DDouble points[] = {{0.47 * size, 0.4 * size}, {0.5 * size, 0.3 * size}, {0.53 * size, 0.4 * size}};
        context->StrokeLines(3, points);
    } else if(shape & Item::RIGHT) {
        const wxPoint2DDouble points[] = {{0.6 * size, 0.47 * size}, {0.7 * size, 0.5 * size}, {0.6 * size, 0.53 * size}};
        context->StrokeLines(3, points);
    } else if(shape & Item::DOWN) {
        const wxPoint2DDouble points[] = {{0.47 * size, 0.6 * size}, {0.5 * size, 0.7 * size}, {0.53 * size, 0.6 * size}};
        context->StrokeLines(3, points);
    } else {
        const wxPoint2DDouble points[] = {{0.4 * size, 0.47 * size}, {0.3 * size, 0.5 * size}, {0.4 * size, 0.53 * size}};
        context->StrokeLines(3, points);
    }
    delete context;
    dc.SelectObject(wxNullBitmap);
    cache[key] = bitmap;
    return bitmap;
}
