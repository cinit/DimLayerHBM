//
// Created by kinit on 2022-04-09.
//

#ifndef DISP_TUNER_NATIVES_HOOKPARAMS_H
#define DISP_TUNER_NATIVES_HOOKPARAMS_H

#include <cstdint>
#include <cstring>
#include <type_traits>
#include <tuple>
#include <array>

template<typename ReturnType, typename ... ArgTypes>
class HookParams {
private:
    const void *mCallerAddress = nullptr;
    int mErrno = 0;
    bool mReturnEarly = false;
    bool mIsReturned = false;
    uint64_t mReturnResult = 0;
    std::array<uint64_t, sizeof...(ArgTypes)> mArgs = {};

public:
    HookParams() = default;

    HookParams(const HookParams &) = delete;

    HookParams(HookParams &&) = delete;

    HookParams &operator=(const HookParams &) = delete;

    HookParams &operator=(HookParams &&) = delete;

private:

    // argument types and return type must be literal and can fit in uint64_t
    template<class T>
    static constexpr bool is_valid_type = sizeof(T) <= 8 && (std::is_arithmetic_v<T> || std::is_pointer_v<T>);
    static_assert(is_valid_type<ReturnType>, "Return type must be literal and can fit in uint64_t");
    static_assert((is_valid_type<ArgTypes> && ...), "Argument types must be literal and can fit in uint64_t");

    template<int... Is>
    class seq {
    };

    template<int N, int... Is>
    struct gen_seq : gen_seq<N - 1, N - 1, Is...> {
    };

    template<int... Is>
    struct gen_seq<0, Is...> : seq<Is...> {
    };

    template<int ...Is>
    inline std::tuple<ArgTypes...> makeArgsTupleImpl(seq<Is...>) const noexcept {
        return std::make_tuple<ArgTypes...>(((ArgTypes) mArgs[Is])...);
    }

    template<int N>
    inline void updateArgsTupleRecursiveImpl(const std::tuple<ArgTypes...> &args) noexcept {
        static_assert(N >= 0 && N < sizeof...(ArgTypes), "N must be in range [0, sizeof...(ArgTypes))");
        using T = std::tuple_element_t<N, std::tuple<ArgTypes...>>;
        if constexpr(std::is_pointer_v<T>) {
            mArgs[N] = reinterpret_cast<uint64_t>(std::get<N>(args));
        } else {
            mArgs[N] = static_cast<uint64_t>(std::get<N>(args));
        }
        if constexpr(N > 0) {
            updateArgsTupleRecursiveImpl<N - 1>(args);
        }
    }

public:

    [[nodiscard]] constexpr int getArgumentCount() const noexcept {
        return sizeof...(ArgTypes);
    }

    [[nodiscard]] std::tuple<ArgTypes...> getArgumentsAsTuple() const noexcept {
        return makeArgsTupleImpl(gen_seq<sizeof...(ArgTypes)>{});
    }

    void updateArguments(std::tuple<ArgTypes...> args) noexcept {
        if constexpr(sizeof...(ArgTypes) > 0) {
            updateArgsTupleRecursiveImpl<sizeof...(ArgTypes) - 1>(args);
        }
    }

    void updateArguments(ArgTypes... args) noexcept {
        if constexpr(sizeof...(ArgTypes) > 0) {
            updateArgsTupleRecursiveImpl<sizeof...(ArgTypes) - 1>(std::make_tuple(args...));
        }
    }

    [[nodiscard]] bool isReturnEarly() const noexcept {
        return mReturnEarly;
    }

    [[nodiscard]] bool isReturned() const noexcept {
        return mIsReturned || mReturnEarly;
    }

    [[nodiscard]] ReturnType getResult() const noexcept {
        return static_cast<ReturnType>(mReturnResult);
    }

    void setResult(ReturnType result) noexcept {
        mReturnResult = static_cast<uint64_t>(result);
        mReturnEarly = true;
        mIsReturned = true;
    }

    [[nodiscard]] int getErrno() const noexcept {
        return mErrno;
    }

    void setErrno(int err) noexcept {
        mErrno = err;
    }

    template<int N, typename T = std::tuple_element_t<N, std::tuple<ArgTypes...>>>
    [[nodiscard]] T getArgv() const noexcept {
        static_assert(N >= 0 && N < sizeof...(ArgTypes), "Index out of bounds");
        if constexpr(std::is_pointer_v<T>) {
            return reinterpret_cast<T>(mArgs[N]);
        } else {
            return static_cast<T>(mArgs[N]);
        }
    }

    template<int N, typename T = std::tuple_element_t<N, std::tuple<ArgTypes...>>>
    void setArgv(T arg) noexcept {
        static_assert(N >= 0 && N < sizeof...(ArgTypes), "Index out of bounds");
        if constexpr(std::is_pointer_v<T>) {
            mArgs[N] = reinterpret_cast<uint64_t>(arg);
        } else {
            mArgs[N] = static_cast<uint64_t>(arg);
        }
    }

    [[nodiscard]] const void *getCallerAddress() const noexcept {
        return mCallerAddress;
    }

    void setCallerAddress(const void *address) noexcept {
        mCallerAddress = address;
    }

};


#endif //DISP_TUNER_NATIVES_HOOKPARAMS_H
