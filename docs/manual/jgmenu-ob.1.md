% JGMENU-OB(1)  
% Johan Malm  
% 22 December, 2017

# NAME

jgmenu-ob - convert openbox menu file to jgmenu flavoured CSV

# SYNOPSIS

`jgmenu_run ob` \[--tag=<*tag*>] \[<*file*>]

# DESCRIPTION

`jgmenu_run ob` coverts openbox menu XML to jgmenu flavoured CSV.  

# OPTIONS

<*file*>
:   specify openbox XML file. If not provided,  
       ~/.config/openbox/menu.xml will be used.  

\--tag=<*tag*>
:   specify menu element id value in XML file i.e.:  
       <*menu id="root-menu" label="Openbox 3"*>  
       If unset, "root-menu" is used as this is the default in  
       openbox's menu.xml.  

# EXAMPLES

This program is designed to be run from jgmenu by settings  
`csv_cmd = ob` in the configuration file (jgmenurc).  

It can also be run directly from the command line like this:  

    jgmenu_run ob | jgmenu

    jgmenu --csv-cmd="jgmenu_run ob"
