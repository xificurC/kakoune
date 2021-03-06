decl str docsclient

hook global WinSetOption filetype=man %{
    addhl group man-highlight
    addhl -group man-highlight regex ^\S.*?$ 0:blue
    addhl -group man-highlight regex ^\h+-+[-a-zA-Z_]+ 0:yellow
    addhl -group man-highlight regex [-a-zA-Z_.]+\(\d\) 0:green
    hook window -group man-hooks NormalKey <c-m> man
    set buffer tabstop 8
}

hook global WinSetOption filetype=(?!man).* %{
    rmhl man-higlight
    rmhooks window man-hooks
}

def -hidden -shell-params _man %{ %sh{
    manout=$(mktemp /tmp/kak-man-XXXXXX)
    colout=$(mktemp /tmp/kak-man-XXXXXX)
    MANWIDTH=${kak_window_width} man "$@" > $manout
    retval=$?
    col -b > ${colout} < ${manout}
    rm ${manout}
    if [ "${retval}" -eq 0 ]; then
        echo "${output}" |
        echo "edit! -scratch '*man*'
              exec |cat<space>${colout}<ret>gk
              nop %sh{rm ${colout}}
              set buffer filetype man"
    else
       echo "echo -color Error %{man '$@' failed: see *debug* buffer for details }"
       rm ${colout}
    fi
} }

def -shell-params man %{ %sh{
    [ -z "$@" ] && set -- "$kak_selection"
    echo "eval -try-client %opt{docsclient} _man $@"
} }
