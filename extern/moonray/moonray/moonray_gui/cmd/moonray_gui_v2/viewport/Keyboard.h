// Copyright 2026 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "BijectiveMap.h"
#include "../GuiTypes.h"

#include <GLFW/glfw3.h>
#include <set>

namespace moonray_gui_v2 {

using KeyModPair = std::pair<int, int>;
inline constexpr KeyModPair INVALID_KEYMOD{-1, -1};

// Custom hash function for KeyModPair
// (Needed for our key bindings, which map a KeyModPair to an Action)
struct KeyModPairHash {
    std::size_t operator()(const KeyModPair& p) const {
        return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
    }
};

using KeyBindingMap = BijectiveMap<KeyModPair, Action, KeyModPairHash, std::hash<Action>>;
using ActionBindingMap = BijectiveMap<Action, KeyModPair, std::hash<Action>, KeyModPairHash>;

// Global function to print hotkey bindings for debugging
void printHotkeys();

// ----------------------------------------- KeyboardBindings Class ------------------------------------------------- //
// KeyboardBindings class that maps key/modifier combinations to Actions
// This class contains the default key bindings for the application.
class KeyboardBindings
{
public:
    KeyboardBindings();
    virtual ~KeyboardBindings() = default;

    // ---------------------------------- Getters ---------------------------------------- //

    // Gets the key/mod pair for a given action.
    // Returns INVALID_KEYMOD if the action is not bound.
    KeyModPair getKeyModPair(const Action action) const;

    // Gets the action for a given key/mod combination, checking for any additional,
    // currently-pressed keys that may act as special modifiers.
    // Returns ACTION_NONE if no action is bound to the combination.
    Action getActionFromInput(GLFWwindow* window, const KeyModPair& keyModPair);

    // Gets the action for the given key/mod combination without checking for
    // any extra special modifiers. Returns ACTION_NONE if no action is bound.
    Action getAction(const KeyModPair& keyModPair) const;

    // Gets the action bound to the scroll input
    Action getScrollAction() const { return mScrollAction; }

    // Gets the default key/mod pair for a given action
    KeyModPair getDefaultKeyModPair(const Action action) const;

    // Checks if a key/mod pair is the default binding for an action
    bool isDefaultBinding(const Action action, const KeyModPair& keyModPair) const
    {
        return getDefaultKeyModPair(action) == keyModPair;
    }

    // Check if key is a standard modifier (Shift, Ctrl, Alt, Super)
    bool isStandardModifier(int key) const
    {
        // GLFW modifier keys are defined up to GLFW_MOD_NUM_LOCK (0x0020),
        // which is equivalent to GLFW_KEY_SPACE (32). Therefore, if it's a
        // standard modifier, it must be less than GLFW_KEY_SPACE (the beginning of the glfw key enums).
        return (key < GLFW_KEY_SPACE) &&
               (key & (GLFW_MOD_SHIFT | GLFW_MOD_CONTROL | GLFW_MOD_ALT | GLFW_MOD_SUPER));
    }

    // Creates and returns a map of the current custom key bindings
    KeyBindingMap getCustomBindings() const;

    // ---------------------------------- Setters ---------------------------------------- //
    // Adds a custom key/mod binding for the given action
    void addCustomBinding(const KeyModPair& keyModPair, const Action action);

    // Resets a key/mod binding for the given action to its default
    void resetBindingToDefault(const Action action);

    // ----------------------------------- Debug ---------------------------------------- //
    // Prints the current custom key bindings
    void printCustomBindings() const;

    // Prints all key bindings
    void printKeyBindings() const;

protected:
    Action mScrollAction = ACTION_IMAGE2D_ZOOM;     // Action for scroll input; currently not bindable
    KeyBindingMap mKeyBindings;                     // Maps key/mod pairs to actions. Originally contains default bindings,
                                                    // but custom bindings can be added/removed at runtime.
    KeyBindingMap mKeyDefaults;                     // A copy of the original default key bindings, used to reset
                                                    // custom bindings back to their defaults.
                                                    // These should not be modified after construction.
    std::set<int> mSpecialMods;                     // Set of non-standard modifier keys (i.e. not Shift, Ctrl, Alt)
private:
    // Deletes a key/mod binding for the given action
    void removeBinding(const Action action);
};

// --------------------------------- MayaKeyboardBindings Class ----------------------------------------------------- //
// MayaKeyboardBindings class that overrides default key bindings for Maya-style keyboard input
class MayaKeyboardBindings : public KeyboardBindings
{
public:
    MayaKeyboardBindings();
    ~MayaKeyboardBindings() override {}
};

// ------------------------------- HoudiniKeyboardBindings Class ---------------------------------------------------- //
// HoudiniKeyboardBindings class that overrides default key bindings for Houdini-style keyboard input
class HoudiniKeyboardBindings : public KeyboardBindings
{
public:
    HoudiniKeyboardBindings();
    ~HoudiniKeyboardBindings() override {}
};

// ----------------------------------------- Keyboard Class --------------------------------------------------------- //
// Keyboard class that manages the current keyboard mode and provides action lookup
// This class allows switching between different keyboard modes (e.g., Maya, Houdini, Default).
// In the future, it will support overrides for specific key/mouse combinations.

class Keyboard {
public:
    Keyboard();
    ~Keyboard() {}

    // -------------------------- Key Binding Lookup ---------------------------- // 

    // Gets the key/mod pair for a given action.
    // Returns INVALID_KEYMOD if the action is not bound.
    KeyModPair getKeyModPair(const Action action) const;
    
    // Gets the action corresponding with the current user input,
    // which is composed of the given key/mod combination, plus any
    // additional pressed keys acting as special modifiers, which we check for internally.
    // Returns ACTION_NONE if no action is bound to the combination.
    Action getActionFromInput(GLFWwindow* glfwWindow, int glfwKey, int glfwMod);

    // Gets the action for scroll input
    Action getScrollAction() const;

    // Returns the conflicting action for a key binding (or ACTION_NONE if no conflict)
    Action getBindingConflict(const int glfwKey, const int glfwMod, const Action targetAction) const;

    // Checks if a key binding would cause a conflict
    bool hasBindingConflict(const int glfwKey, const int glfwMod, const Action targetAction) const;

    // Checks if a key/mod pair is the default binding for an action
    bool isDefaultBinding(const Action action, const KeyModPair& keyModPair) const
    {
        return mBindings->isDefaultBinding(action, keyModPair);
    }

    // -------------------------- Setters ---------------------------- //
    void setDefaultKeyboardMode() { changeKeyboardMode(KeyboardMode::Default); }
    void setMayaKeyboardMode()    { changeKeyboardMode(KeyboardMode::Maya); }
    void setHoudiniKeyboardMode() { changeKeyboardMode(KeyboardMode::Houdini); }

    // Override a key binding (returns true if successful, false if there's a conflict)
    bool addCustomBinding(const int glfwKey, int glfwMod, Action action, 
                          Action& conflictingAction, bool forceOverride = false);

    // Resets a key binding for the given action to its default value
    bool resetBindingToDefault(const Action action, Action& conflictingAction, 
                               KeyModPair& defaultKeyModPair, bool forceOverride = false);

    // Resets all key bindings to their default values
    void resetToDefaults();

    // ------------------------- Key Binding Persistence ------------------------- //
    // Saves the current key bindings to a file
    bool saveKeyBindings(const std::string filename) const;

    // Loads key bindings from a file
    bool loadKeyBindings(const std::string filename);

    // Prints the current key bindings to the console
    void printKeyBindings() const { mBindings->printKeyBindings(); }

private:
    enum class KeyboardMode {
        Default,
        Maya,
        Houdini
    };

    // Internal method to change keyboard mode while preserving custom bindings
    void changeKeyboardMode(const KeyboardMode mode);

    std::unique_ptr<KeyboardBindings> mBindings;                                 // keybindings map
    KeyboardMode mKeyboardMode {KeyboardMode::Default};                          // current keyboard mode
};

} // end namespace moonray_gui_v2
