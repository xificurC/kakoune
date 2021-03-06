#ifndef context_hh_INCLUDED
#define context_hh_INCLUDED

#include "selection.hh"
#include "optional.hh"
#include "flags.hh"

namespace Kakoune
{

class Window;
class Buffer;
class Client;
class Scope;
class InputHandler;
class UserInterface;
class DisplayLine;
class KeymapManager;
class AliasRegistry;

struct Disableable
{
    void disable() { m_disable_count++; }
    void enable() { kak_assert(m_disable_count > 0); m_disable_count--; }
    bool is_disabled() const { return m_disable_count > 0; }
    bool is_enabled() const { return m_disable_count == 0; }
private:
    int m_disable_count = 0;
};

struct ScopedDisable
{
    ScopedDisable(Disableable& disableable, bool condition = true)
        : m_disableable(disableable), m_condition(condition)
    {
        if (m_condition)
            m_disableable.disable();
    }

    ~ScopedDisable()
    {
        if (m_condition)
            m_disableable.enable();
    }

private:
    Disableable& m_disableable;
    bool m_condition;
};

// A Context is used to access non singleton objects for various services
// in commands.
//
// The Context object links a Client, a Window, an InputHandler and a
// SelectionList. It may represent an interactive user window, a hook
// execution or a macro replay context.
class Context
{
public:
    enum class Flags
    {
        None = 0,
        Transient = 1,
    };

    Context(InputHandler& input_handler, SelectionList selections,
            Flags flags, String name = "");

    struct EmptyContextFlag {};
    explicit Context(EmptyContextFlag);
    ~Context();

    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;

    Buffer& buffer() const;
    bool has_buffer() const { return (bool)m_selections; }

    Window& window() const;
    bool has_window() const { return (bool)m_window; }

    Client& client() const;
    bool has_client() const { return (bool)m_client; }

    InputHandler& input_handler() const;
    bool has_input_handler() const { return (bool)m_input_handler; }

    UserInterface& ui() const;
    bool has_ui() const { return has_client(); }

    SelectionList& selections();
    const SelectionList& selections() const;
    Vector<String>  selections_content() const;

    // Return potentially out of date selections
    SelectionList& selections_write_only();

    void change_buffer(Buffer& buffer);

    void set_client(Client& client);
    void set_window(Window& window);

    Scope& scope() const;

    OptionManager& options() const;
    HookManager& hooks() const;
    KeymapManager& keymaps() const;
    AliasRegistry& aliases() const;

    void print_status(DisplayLine status) const;

    StringView main_sel_register_value(StringView reg) const;

    void push_jump();
    const SelectionList& jump_forward();
    const SelectionList& jump_backward();
    void forget_jumps_to_buffer(Buffer& buffer);

    const String& name() const { return m_name; }
    void set_name(String name) { m_name = std::move(name); }

    bool is_editing() const { return m_edition_level!= 0; }
    void disable_undo_handling() { m_edition_level = -1; }

    Disableable& user_hooks_support() { return m_user_hooks_support; }
    const Disableable& user_hooks_support() const { return m_user_hooks_support; }

    Disableable& keymaps_support() { return m_keymaps_support; }
    const Disableable& keymaps_support() const { return m_keymaps_support; }

    Disableable& history_support() { return m_history_support; }
    const Disableable& history_support() const { return m_history_support; }

    Flags flags() const { return m_flags; }

private:
    void begin_edition();
    void end_edition();
    int m_edition_level = 0;

    friend struct ScopedEdition;

    Flags m_flags;

    SafePtr<InputHandler> m_input_handler;
    SafePtr<Window>       m_window;
    SafePtr<Client>       m_client;

    Optional<SelectionList> m_selections;

    String m_name;

    using JumpList = Vector<SelectionList>;
    JumpList           m_jump_list;
    JumpList::iterator m_current_jump = m_jump_list.begin();

    Disableable m_user_hooks_support;
    Disableable m_keymaps_support;
    Disableable m_history_support;
};

template<>
struct WithBitOps<Context::Flags> : std::true_type {};

struct ScopedEdition
{
    ScopedEdition(Context& context)
        : m_context(context), m_buffer(&context.buffer())
    { m_context.begin_edition(); }

    ~ScopedEdition() { m_context.end_edition(); }

    Context& context() const { return m_context; }
private:
    Context& m_context;
    SafePtr<Buffer> m_buffer;
};

}
#endif // context_hh_INCLUDED
