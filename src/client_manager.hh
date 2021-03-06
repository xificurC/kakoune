#ifndef client_manager_hh_INCLUDED
#define client_manager_hh_INCLUDED

#include "client.hh"
#include "completion.hh"

namespace Kakoune
{

struct client_removed{};

struct WindowAndSelections
{
    std::unique_ptr<Window> window;
    SelectionList selections;
    size_t timestamp;
};

class ClientManager : public Singleton<ClientManager>
{
public:
    ClientManager();
    ~ClientManager();

    Client* create_client(std::unique_ptr<UserInterface>&& ui,
                          EnvVarMap env_vars, StringView init_cmd);

    bool   empty() const { return m_clients.empty(); }
    size_t count() const { return m_clients.size(); }

    void    ensure_no_client_uses_buffer(Buffer& buffer);

    WindowAndSelections get_free_window(Buffer& buffer);
    void add_free_window(std::unique_ptr<Window>&& window, SelectionList selections);

    void redraw_clients() const;
    void handle_pending_inputs() const;

    Client*  get_client_ifp(StringView name);
    Client&  get_client(StringView name);
    bool validate_client_name(StringView name) const;
    void remove_client(Client& client);

    using ClientList = Vector<std::unique_ptr<Client>, MemoryDomain::Client>;
    using iterator = ClientList::const_iterator;

    iterator begin() const { return m_clients.begin(); }
    iterator end() const { return m_clients.end(); }

    CandidateList complete_client_name(StringView name,
                                       ByteCount cursor_pos = -1) const;

private:
    String generate_name() const;

    ClientList m_clients;
    Vector<WindowAndSelections, MemoryDomain::Client> m_free_windows;
};

}

#endif // client_manager_hh_INCLUDED
