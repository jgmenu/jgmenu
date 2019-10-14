% JGMENUUNICODE(7)  
% Johan Malm  
% 20 September, 2019  

# NAME

jgmenuunicode - An overview of jgmenu unicode usage

# INTRODUCTION

Unicode characters can be displayed by jgmenu. They can be used in:

- CSV data  
- Widgets  
- Config file  

Throughout this document, unicode code points will be referred to by writing
`u+` folled by their hexadecimal number. In examples, u+1234 will be used.

# UNICODE CHARACTERS IN TERMINAL

Terminals handle unicode differently. For full support, avoid xterm and urxvt,
and opt for something libvte based such as sakura or gnome-terminal. In a
terminal, you can produce a unicode character by issuing

    printf '%b' '\\u1234'

In libvte terminals, you can press ctrl+shift+u and then enter the hex sequence.

# UNICODE CHARACTERS IN VIM

Whilst in insert mode, press ctrl+V and then type u1234.

Use the command `ga` to show the hex value of the character under the cursor.

# VARIATION SELECTORS

Some unicode characters can be rendered as either emoji or text. Special
invisible unicode characters called variation selectors can be used to force
either presentation.  Append with the text presentation selector (u+fe0e) or
emoji presentation selector (u+fe0f), to force text or emoji respectively. For
example:

u+1f50d u+fe0f  🔍  
u+1f50d u+fe0e  🔍︎  

# USEFUL CHARACTERS FOR BUILDING MENUS

## SEARCH

u+2315  ⌕  
u+26b2  ⚲  
u+1c04  ᰄ  

## ARROWS

u+2192 →  
u+203a ›  
u+25b6 ▶  
u+2794 ➔  
u+2799 ➙  
u+279b ➛  
u+279c ➜  
u+279d ➝  
u+279e ➞  
u+279f ➟  
u+27a0 ➠  
u+27a1 ➡  
u+27a2 ➢  
u+27a3 ➣  
u+27a4 ➤  
u+27a5 ➥  
u+27a6 ➦  
u+21a6 ↦  
u+21d2 ⇒  
u+21dd ⇝  
u+21e2 ⇢  
u+21e5 ⇥  
u+21e8 ⇨  
u+21fe ⇾  
u+27ad ➭  
u+27ae ➮  
u+27af ➯  
u+27b1 ➱  
u+27b2 ➲  
u+27ba ➺  
u+27bc ➼  
u+27bd ➽  
u+27be ➾  
