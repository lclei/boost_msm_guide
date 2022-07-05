#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>
#include <boost/msm/front/functor_row.hpp>
#include <iostream>

namespace {
    template<typename T>
    std::string get_typename(const T &x) {
        std::string name = boost::core::demangle(typeid(x).name());
        auto pos = name.rfind("::");
        if (pos != std::string::npos) {
            name = name.substr(pos + 2);
        }
        return name;
    }

    struct Context {
        int val;
    };

    struct BaseState : boost::msm::front::state<> {
        virtual ~BaseState() {}
        template<class Event, class Fsm>
        void on_entry(Event const &event, Fsm &fsm) {
            std::cout << "...Entering " << get_typename(*this)
                    << " by " << get_typename(event) << "\n";
            prepare(fsm.ctx);
        }
        template<class Event, class Fsm>
        void on_exit(Event const &event, Fsm &fsm) {
            std::cout << "...Leaving " << get_typename(*this)
                    << " by " << get_typename(event) << "\n";
            clean(fsm.ctx);
        }
        virtual void prepare(Context &) {}
        virtual void clean(Context &) {}
    };

    // ---Events
    struct Event1 {};

    // ---State Machine front-end
    struct Sm : boost::msm::front::state_machine_def<Sm>
    {
        // States
        struct State1 : BaseState {};

        struct End : boost::msm::front::terminate_state<> {};

        // Set initial state
        using initial_state = State1;

        // Transition Table
        struct transition_table : boost::mpl::vector<
            //    Start | Event | Next | Action | Guard
            _row< State1, Event1, End                  >
        > {};

        Context ctx;
    };

    // Pick a back-end
    using Fsm = boost::msm::back::state_machine<Sm>;

    void test() {
        Fsm fsm;
        fsm.start();
        std::cout << "> Send Event1\n";
        fsm.process_event(Event1{});
    }
}

int main() {
    test();
    return 0;
}