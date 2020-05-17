#pragma once

class CompAssets
{
public:
    CompAssets(
        winrt::Windows::UI::Composition::Compositor const& compositor,
        winrt::Windows::Foundation::Numerics::float2 const& tileSize);
    ~CompAssets() {}

    winrt::Windows::UI::Composition::CompositionColorBrush GetMineBrush() { return m_mineBrush; }
    winrt::Windows::UI::Composition::CompositionColorBrush GetColorBrushFromMineState(MineState state);
    winrt::Windows::UI::Composition::CompositionColorBrush GetColorBrushFromMineCount(int count);
    winrt::Windows::UI::Composition::CompositionShape GetShapeFromMineCount(int count);

private:
    void GenerateAssets(
        winrt::Windows::UI::Composition::Compositor const& compositor,
        winrt::Windows::Foundation::Numerics::float2 const& tileSize);

private:
    winrt::Windows::UI::Composition::CompositionColorBrush m_mineBrush{ nullptr };
    std::map<MineState, winrt::Windows::UI::Composition::CompositionColorBrush> m_mineStateBrushes;
    std::map<int, winrt::Windows::UI::Composition::CompositionColorBrush> m_mineCountBackgroundBrushes;
    std::map<int, winrt::Windows::UI::Composition::CompositionShape> m_mineCountShapes;
};