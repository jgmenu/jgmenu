% JGMENU-OB(1)  
% Johan Malm  
% 15 Jan, 2019  

# NAME

jgmenu-ob - convert openbox menu data to jgmenu flavoured CSV  

# SYNOPSIS

`jgmenu_run ob` \[\-\-tag=<*tag*>] \[\-\-cmd=<*cmd*> | <*file*>]  

# DESCRIPTION

`jgmenu_run ob` coverts openbox XML menu data to jgmenu flavoured  
CSV. If no <*file*> or --cmd=<*cmd*> is specified,  
~/.config/openbox/menu.xml or /etc/xdg/openbox/menu.xml will be used  
if they exist (giving higher precedence to the former).  

If environment variable `JGMENU_I18N` or config variable `csv_i18n`  
are set, a translation file will be searched for in the specified  
file or directory. See `jgmenu_run i18n --help` for further details.  

# OPTIONS

<*file*>
:   specify openbox XML file.  

\--cmd=<*cmd*>
:   specify command to produce openbox XML menu data.  

\--tag=<*tag*>
:   specify menu element id value in XML file i.e.:  
       <*menu id="root-menu" label="Openbox 3"*>  
       If unset, "root-menu" is used as this is the default in  
       openbox's menu.xml.  

# EXAMPLES

This program is designed to be run from jgmenu by settings  
`csv_cmd = ob` in the configuration file (jgmenurc).  

It can also be run directly from the command line like this:  

    jgmenu_run ob | jgmenu --simple

    jgmenu --csv-cmd="jgmenu_run ob"
