// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <tinyfsm/include/tinyfsm.hpp>
#include <iostream>

using Event = tinyfsm::Event;

template<class T>
using Fsm = tinyfsm::Fsm<T>;

#include <iostream>

struct Off;  // forward declaration

// ----------------------------------------------------------------------------
// 1. Event Declarations
//
struct Toggle : Event {};

// ----------------------------------------------------------------------------
// 2. State Machine Base Class Declaration
//
struct Switch : Fsm<Switch> {
    virtual void react(Toggle const &){};

    // alternative: enforce handling of Toggle in all states (pure virtual)
    // virtual void react(Toggle const &) = 0;

    virtual void entry(void){}; /* entry actions in some states */
    void exit(void){};          /* no exit actions */

    // alternative: enforce entry actions in all states (pure virtual)
    // virtual void entry(void) = 0;
};

// ----------------------------------------------------------------------------
// 3. State Declarations
//
struct On : Switch {
    void entry() override { std::cout << "* Switch is ON" << std::endl; };
    void react(Toggle const &) override { transit<Off>(); };
};

struct Off : Switch {
    void entry() override { std::cout << "* Switch is OFF" << std::endl; };
    void react(Toggle const &) override { transit<On>(); };
};

FSM_INITIAL_STATE(Switch, Off)
