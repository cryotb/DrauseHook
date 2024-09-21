#pragma once

namespace rs
{
    enum class InKeyStates : int
    {
        UNSET = 0,
        FORCE = 1,
        TAP = 2,
        RELEASE = 3,
        IDLE = 4,
        HOLDING = 5,
    };

    class In_state
    {
    public:
        In_state()
        {
            _pstate = nullptr;
        }

        In_state(uptr addr) :
            In_state()
        {
            construct(addr);
        }

        void construct(uptr addr)
        {
            _pstate = reinterpret_cast<InKeyStates *>(addr);
        }

        auto state() const { return *_pstate; }

        void tap() const
        {
            *_pstate = InKeyStates::TAP;
        }

        void force() const
        {
            *_pstate = InKeyStates::FORCE;
        }

        void toggle(bool val) const
        {
            if (val)
                *_pstate = InKeyStates::HOLDING;
            else
                *_pstate = InKeyStates::IDLE;
        }

        void block() const
        {
            *_pstate = InKeyStates::RELEASE;
        }

        void reset() const
        {
            *_pstate = InKeyStates::IDLE;
        }

        bool active() const
        {
            if (*_pstate == InKeyStates::HOLDING || *_pstate == InKeyStates::FORCE ||
                *_pstate == InKeyStates::TAP)
                return true;

            return false;
        }

    private:
        InKeyStates *_pstate;
    };
}
