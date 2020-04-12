# DankBSD Default zshrc

source /usr/local/etc/zsh-syntax-highlighting/zsh-syntax-highlighting.sh

# Enable aliases to be sudo’ed
alias sudo="sudo "
alias doas="doas "

# Navigation
alias p="popd"
alias ..="cd .."
alias ...="cd ../.."
alias ....="cd ../../.."
alias .....="cd ../../../.."
alias l1="tree --dirsfirst -ChFL 1"
alias l2="tree --dirsfirst -ChFL 2"
alias l3="tree --dirsfirst -ChFL 3"
alias l4="tree --dirsfirst -ChFL 4"
alias l5="tree --dirsfirst -ChFL 5"
alias l="l1"
alias la="ls -lAFh" # long list, show all except . and .., show type, human readable

mcd() { mkdir -p $1 && cd $1; }

fancy-ctrl-z() {
	if [[ $#BUFFER -eq 0 ]]; then
		BUFFER=fg
		zle accept-line
	else
		zle push-input
		zle clear-screen
	fi
}
zle -N fancy-ctrl-z && bindkey '^Z' fancy-ctrl-z

autoload -U colors && colors
autoload -U compinit && zmodload -i zsh/complist
autoload -U edit-command-line && zle -N edit-command-line
autoload -U url-quote-magic && zle -N self-insert url-quote-magic
autoload -U zmv

zmodload zsh/terminfo
bindkey -e # Emacs style keys in shell
bindkey '^xe' edit-command-line
bindkey '^x^e' edit-command-line
bindkey "\e[3~" delete-char # Del

setopt pushd_ignore_dups auto_pushd auto_name_dirs auto_cd \
	prompt_subst no_beep multios extended_glob interactive_comments

setopt hist_ignore_dups hist_ignore_space hist_reduce_blanks hist_verify \
	hist_expire_dups_first hist_find_no_dups share_history extended_history \
	append_history inc_append_history nobanghist

setopt menu_complete # Autoselect the first suggestion
setopt complete_in_word
setopt no_complete_aliases # Actually: completes aliases! (I guess that means "no ~separate functions~ for aliases")
unsetopt always_to_end
zstyle ':completion:*' squeeze-slashes true
zstyle ':completion:*' insert-tab pending
zstyle ':completion:*' expand "yes"
zstyle ':completion:*' matcher-list "m:{a-zA-Z}={A-Za-z}" # ignore case
zstyle ':completion:*' list-colors ""
zstyle ':completion:*' menu select=2 _complete _ignored _approximate
zstyle ':completion:*' group-name ''
zstyle ':completion:*' verbose yes
zstyle ':completion:*:default' list-prompt '%S%M matches%s'
zstyle ':completion:*:prefix:*' add-space true
zstyle ':completion:*:descriptions' format "|| %{${fg[yellow]}%}%d%{${reset_color}%}"
zstyle ':completion:*:messages' format $'\e[00;31m%d'
zstyle ':completion:*:manuals' separate-sections true
zstyle ':completion:*:manuals.(^1*)' insert-sections true
zstyle ':completion:*::::' completer _expand _complete _ignored _approximate
zstyle ':completion:*:match:*' original only
zstyle ':completion:*:approximate:*' max-errors 1 numeric
zstyle ':completion:*:cd:*' ignore-parents parent pwd
zstyle ':completion:*:rm:*' ignore-line yes
zstyle ':completion:*:*:*:processes' list-colors "=(#b) #([0-9]#) ([0-9a-z-]#)*=01;34=0=01"
zstyle ':completion:*:*:*:processes' command "ps -u $USER -o pid,user,comm -w -w"
zstyle ':completion:*:*:*:*:hosts' list-colors "=*=$color[cyan];$color[bg-black]"
zstyle ':completion:*:functions' ignored-patterns "_*"
zstyle ':completion:*:original' list-colors "=*=$color[red];$color[bold]"
zstyle ':completion:*:parameters' list-colors "=[^a-zA-Z]*=$color[red]"
zstyle ':completion:*:aliases' list-colors "=*=$color[green]"
compinit -i
compdef mcd=cd

HISTSIZE=40960
SAVEHIST=40960
setopt HIST_EXPIRE_DUPS_FIRST
WORDCHARS='*?_-.[]~=&;!#$%^(){}<>' # Like default, but without / -- ^W must be useful in paths, like it is in vim, bash, tcsh
PROMPT="%B%F{cyan}%~%b %F{yellow}%#%f "
RPROMPT="%(0?..%F{red}^%?^) %F{blue}%T%f"
