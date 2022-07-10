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


    // ---Events
    struct Event1 {};
    struct Event2 {};
    struct Event3 {};
    struct Event4 {};
    struct Event5 {};
    struct Event6 {};

    void sth_wrong() {
        std::vector<int> a(1);
        int i = a.at(2);
    }

    // ---State Machine front-end
    struct Sm : boost::msm::front::state_machine_def<Sm>
    {
        // ---States
        struct State1 : BaseState {};
        struct State2 : BaseState {};
        struct Init : BaseState {};
        struct End : boost::msm::front::terminate_state<> {};
        struct ExceptionOnEntry : BaseState {
            void prepare(Context &) override {
                sth_wrong();
            }
        };
        struct ExceptionOnExit : BaseState {
            void clean(Context &) override {
                sth_wrong();
            }
        };

        // ---Actions
        struct Action1 : BaseAction {};
        struct Action2 : BaseAction {};
        struct ExceptionAction : BaseAction {
            void execute(Context &) override {
                sth_wrong();
            }
        };

        // ---Guard
        struct Guard1 : BaseGuard {};
        struct GTrue : BaseGuard {};
        struct GFalse : BaseGuard {
            bool execute(Context &) override {
                return false;
            }
        };
        struct ExceptionGuard : BaseGuard {
            bool execute(Context &) override {
                sth_wrong();
                return true;
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
            Row< State1, Event1, ExceptionOnEntry, Action1, GTrue  >,
            Row< State1, Event2, State2, ExceptionAction, GTrue >,
            Row< State1, Event3, State2, Action1, ExceptionGuard >,
            Row< State1, Event4, ExceptionOnExit, Action1, GTrue >,
            Row< ExceptionOnExit, Event5, State1, Action1, GTrue >
        > {};

        template<class Event, class Fsm>
        void exception_caught (Event const& evt, Fsm& fsm,std::exception& e) {
            std::cout << "catch exception " << e.what() << " while " << get_typename(evt)
                << '\n';
        }
        template<class Event, class Fsm>
        void no_transition(Event const& evt, Fsm &fsm, int x) {
            std::cout << "No transition item for " << x
                << " while " << get_typename(evt) << '\n';
        }

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
        std::cout << "> Send Event3\n";
        fsm.process_event(Event3{});
        std::cout << "> Send Event4\n";
        fsm.process_event(Event4{});
        std::cout << "> Send Event5\n";
        fsm.process_event(Event5{});
        std::cout << "> Send Event1\n";
        fsm.process_event(Event1{});
    }
}

int main() {
    test();
    return 0;
}