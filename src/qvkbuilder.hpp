#pragma once

namespace QVkBuilder
{

    template <unsigned int N> class SubPass
    {
    public:
        static const unsigned int maxAttachments = N;
    };

    template <typename... Args> struct EnumSubPasses;
    template <typename Arg> struct EnumSubPasses<Arg>
    {
        static const unsigned int NumReferences = Arg::maxAttachments;
        static const unsigned int NumSupbasses = 1;
    };
    template <typename Arg, typename... Args> struct EnumSubPasses<Arg, Args...>
    {
        typedef EnumSubPasses<Args...> next;
        static const unsigned int NumReferences = next::NumReferences + Arg::maxAttachments;
        static const unsigned int NumSupbasses = next::NumSupbasses + 1;
    };

    template <unsigned int N, typename... Args> class RenderPass : EnumSubPasses<Args...>
    {
    };

}
