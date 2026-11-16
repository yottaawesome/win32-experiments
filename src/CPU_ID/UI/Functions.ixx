export module CpuId:UI.Functions;

export namespace UI
{
    auto NextControlId() -> unsigned
    {
        static auto nextId = 1u;
        return nextId++;
    }
}
