#pragma once
enum class MineState
{
    Empty = 0,
    Flag = 1,
    Question = 2,
    Revealed = 3
};

enum class MineGenerationState
{
    Deferred,
    Generated,
};

class IMinesweeper
{
public:
    virtual ~IMinesweeper() {}

    virtual void OnPointerMoved(winrt::Windows::Foundation::Numerics::float2 point) = 0;
    virtual void OnParentSizeChanged(winrt::Windows::Foundation::Numerics::float2 newSize) = 0;
    virtual void OnPointerPressed(
        bool isRightButton,
        bool isEraser) = 0;
};

std::shared_ptr<IMinesweeper> CreateMinesweeper(
    winrt::Windows::UI::Composition::ContainerVisual parentVisual,
    winrt::Windows::Foundation::Numerics::float2 parentSize);
