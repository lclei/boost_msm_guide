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
            std::cout << "[state]...Entering " << get_typename(*this)
                    << " by " << get_typename(event) << "\n";
            prepare(fsm.ctx);
        }
        template<class Event, class Fsm>
        void on_exit(Event const &event, Fsm &fsm) {
            std::cout << "[state]...Leaving " << get_typename(*this)
                    << " by " << get_typename(event) << "\n";
            clean(fsm.ctx);
        }
        virtual void prepare(Context &) {}
        virtual void clean(Context &) {}
    };

    struct BaseAction {
        virtual ~BaseAction() {}
        template<class Event, class Fsm, class SourceState, class TargetState>
        void operator()(Event const &event, Fsm &fsm, SourceState const &from, TargetState const &to) {
            std::cout << "[action] Do " << get_typename(*this) << " from "
                    << get_typename(from) << " to " << get_typename(to)
                    << " by " << get_typename(event) << "\n";
            execute(fsm.ctx);
        }
        virtual void execute(Context &) {}
    };

    struct BaseGuard {
        virtual ~BaseGuard() {}
        template<class Event, class Fsm, class SourceState, class TargetState>
        bool operator()(Event const &event, Fsm &fsm, SourceState const &from, TargetState const &to) {
            bool ok = execute(fsm.ctx);
            std::cout << "[guard] " << get_typename(*this) << " -> " << std::boolalpha << ok
                    << " from " << get_typename(from) << " to " << get_typename(to)
                    << " by " << get_typename(event) << "\n";
            return ok;
        }
        virtual bool execute(Context &) {return true;}
    };


    // ---State Machine front-end
    struct Sm : boost::msm::front::state_machine_def<Sm>
    {
        // ---States
        struct State1 : BaseState {};
        struct State2 : BaseState {};
        struct Init : BaseState {};
        struct End : boost::msm::front::terminate_state<> {};

        // ---Events
        struct Event1 {};
        struct Event2 {};

        // ---Actions
        struct Action1 : BaseAction {};
        struct Action2 : BaseAction {};

        // ---Guard
        struct Guard1 : BaseGuard {};
        struct GTrue : BaseGuard {};
        struct GFalse : BaseGuard {
            bool execute(Context &) override {
                return false;
            }
        };

        // ---Set initial state
        using initial_state = Init;

        template<typename... T>
        using Row = boost::msm::front::Row<T...>;
        using None = boost::msm::front::none;

        // ---Transition Table
        struct transition_table : boost::mpl::vector<
            //   Start * Event * Next  * Action * Guard
            Row< Init,   None,   State1, None,    None   >,
            Row< State1, Event1, State2, Action1, Guard1 >,
            Row< State2, Event2, End,    Action2, None   >
        > {};

        Context ctx;
    };

    // ---Pick a back-end
    using Fsm = boost::msm::back::state_machine<Sm>;

    void test() {
        Fsm fsm;
        fsm.start();
        std::cout << "> Send Event1\n";
        fsm.process_event(Event1{});
        std::cout << "> Send Event2\n";
        fsm.process_event(Event2{});
    }
}

int main() {
    test();
    return 0;
}