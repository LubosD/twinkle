# Twinkle

Twinkle is a SIP-based VoIP client.

## Dependencies

To compile Twinkle you need the following libraries:

* ucommon [GNU uCommon C++](http://www.gnu.org/software/commoncpp/)
* ccRTP (version >= 1.5.0) [GNU RTP Stack](http://www.gnu.org/software/ccrtp/)
* libxml2
* libsndfile
* libmagic
* libreadline
* Qt 5 -- more specifically, the following submodules:
  * base
  * declarative
  * tools

The following tools are also required:

* cmake
* bison
* flex

### Optional dependencies

* alsa-lib (also known as libasound)
* libzrtpcpp (version >= 0.9.0) [ZRTP library, ccRTP support must be enabled](http://www.gnutelephony.org/index.php/GNU_ZRTP)
* bcg729 [G.729A codec library](http://www.linphone.org/technical-corner/bcg729/overview)
* Speex and SpeexDSP [Speex codec library](http://www.speex.org/)
* iLBC [iLBC codec library](http://www.ilbcfreeware.org/)

## Build

First of all, choose which options you want to have enabled.

All possible options are:

* Qt 5 GUI: `-DWITH_QT5=On` (on by default)
* D-Bus use: `-DWITH_DBUS=On` (on by default, requires `WITH_QT5`)
* ALSA support: `-DWITH_ALSA=On` (on by default)
* ZRTP support: `-DWITH_ZRTP=On`
* G.729A codec support: `-DWITH_G729=On`
* Speex codec support: `-DWITH_SPEEX=On`
* iLBC codec support: `-DWITH_ILBC=On`
* Diamondcard support: `-DWITH_DIAMONDCARD=On`

### Build instructions

	# Create a subdirectory for the build an enter it
	mkdir build && cd build

	# Run cmake with a list of build options
	cmake .. -Dexample_option=On

	# Build Twinkle
	make

	# Install Twinkle
	make install

## Shared user data

Installation will create the following directory for shared user data
on your system:

	${CMAKE_INSTALL_PREFIX}/share/twinkle

The typical default value for `CMAKE_INSTALL_PREFIX` is `/usr/local`.

## Application icon

If you want to create an application link on your desktop you
can find an application icon in the shared user data directory:

*	twinkle16.png	(16x16 icon)
*	twinkle32.png	(32x32 icon)
*	twinkle48.png	(48x48 icon)

## User data

On first run Twinkle will create the `.twinkle` directory in your home
directory. In this directory all user data will be put:

* user profiles (.cfg)
* log files (.log)
* system settings (twinkle.sys)
* call history (twinkle.ch)
* lock file (twinkle.lck)

## Starting Twinkle

Give the command: twinkle

`twinkle -h` will show you some command line options you may use.

NOTE: the CLI option is not fool proof. A command given at a wrong
      time may crash the program. It is recommended to use the GUI.
  
If you do not specify a configuration file (`-f <profile>`) on the command
line, then Twinkle will look for configuration files in your 
`.twinkle` directory. 

If you do not have any configuration file, the configuration file
editor will startup so you can create one. If you have
configuration files, then Twinkle lets you select an 
existing configuration file. See below for some hints on
settings to be made with the profile configuration editor.

If you specify a configuration file name, then Twinkle will
such for this configuration file in your .twinkle directory.
If you have put your configuration file in another location
you have to specify the full path name for the file, i.e.
starting with a slash.

NOTE: the configuration file editor only exists in the GUI.
      If you run the CLI mode, you must have a configuration file.
      So first create a configuration file in GUI mode or hand edit
      a configuration file, before running the CLI mode.
      If you run the CLI mode and you do not specify a file name
      on the command line, then Twinkle will use twinkle.cfg

## NAT

If there is a NAT between you and your SIP server then you have
3 options to make things work:

1. Your SIP provider uses a Session Border Controller
2. Your SIP provider offers a STUN server
3. Make static address mappings in your NAT for SIP and RTP

STUN can be enabled in the NAT section of the user profile.

For the static address mappings enable the following in
the NAT section of the user profile:

      Use statically configured public IP address inside SIP messages

And fill in the public IP address of your NAT.

Twinkle will then use this IP address inside SIP headers and
SDP bodies instead of the private IP address of your machine.

In addition you have to add the following port forwardings for UDP
on your NAT

	public:5060 --> private:5060 (for SIP signaling)
	public:8000 --> private:8000 (for RTP on line 1)
	public:8001 --> private:8001 (for RTCP on line 1)
	public:8002 --> private:8002 (for RTP on line 2)
	public:8003 --> private:8003 (for RTCP on line 2)
    public:8004 --> private:8004 (for RTP for call transfer)
    public:8005 --> private:8005 (for RTCP for call transfer)

If you have changed the SIP/RTP ports in your profile you have
to change the port forwarding rules likewise.

## Log files

During execution Twinkle will create the following log files in
your .twinkle directory:

* twinkle.log (latest log file)
* twinkle.log.old (previous log file)

When twinkle.log is full (default is 5 MB) then it is moved to 
twinkle.log.old and a new twinkle.log is created.

On startup an existing twinkle.log is moved to twinkle.log.old and a
new twinkle.log is created.

## User profile configuration

A user profile contains information about your user account,
SIP proxy, and several SIP protocol options. If you use Twinkle
with different user accounts you may create multiple user
profiles.

When you create a new profile you first give it a name and
then you can make the appropriate settings. The name of the
profile is what later on appears in the selection box
when you start Twinkle again. Or you can give the name.cfg
at the command line (-f option) to immediately start that
profile.

The user profile is stored as '<name>.cfg' in the .twinkle
directory where <name> is the name you gave to the profile.

At a minimum you have to specify the following:

* User name: this is your SIP user name (eg. phone number)
* Domain:    the domain of your provider (eg. fwd.pulver.com)
             this could also be the IP address of your SIP proxy
	     if you want to do IP-to-IP dialing (without proxy) then
	     fill in the IP address or FQDN of your computer.

If your SIP proxy does not request authentication and the value you
filled in for 'Domain' can be resolved to an IP address by Twinkle,
eg. it is an IP address or an FQDN that is in an A-record of the
DNS, then you are ready now.

## Authentication

If your proxy needs authentication, then specify the following fields
in the SIP authentication box:

* Realm:	the realm for authentication
	 	you might leave the realm empty. If you do so, then
		Twinkle will use the name and password regardless of
		the realm put in the challenge by the proxy. For most
		network setups this is fine. You only need to explicitly
		specify a realm when you have call scenario's where
		you have to access multiple realms. Then for the realms
		not known to Twinkle you will be requested for a login
		when needed.
* Name:		your authentication name
* Password:	your authentication password

If authentication fails during registration or any other SIP request
because you filled in wrong values, then Twinkle will at that time
interactively request your login and cache it.

## Outbound proxy

An outbound proxy is only needed if the domain value cannot be resolved
to an IP address by Twinkle or because your provider demands you to
use an outbound proxy that is at a different IP address.

Check the 'use outbound proxy' check box in the SIP server section.
For outbound proxy fill in an IP address or an FQDN that can be
resolved to an IP address via DNS.

By default only out-of-dialog requests (eg. REGISTER, OPTIONS, initial
INVITE) are sent to the outbound proxy. In-dialog requests (eg. re-INVITE,
BYE) are sent to the target indicated by the far end during call setup.
By checking 'send in-dialog requests to proxy' Twinkle will ignore this
target and send these requests also to the proxy. Normally you would
not need this. It could be useful in a scenario where the far-end
indicates a target that cannot be resolved to an IP address.

By checking "Do not send a request to proxy if its destination can be
resolved locally" will make Twinkle always first try to figure out
the destination IP address itself, i.e. based on the request-URI and
Route-headers. Only when that fails the outbound-proxy will be tried,
but only for the options checked above. I.e. if you did not check
the 'in-dialog' option, then an in-dialog request will
never go to the proxy. If its destination cannot be resolved, then
the request will simply fail.

## Registrar

By default a REGISTER will be send to the IP address resolved from
the domain value or to the outbound proxy if specified.

If your service provider has a dedicated registrar which is
different from these IP addresses, then you can specify the
IP or FQDN of the registrar in the registrar-field.

The 'expiry' value is the expiry of your registration. Just before
the registration expires Twinkle will automatically refresh the
registration. The expiry time may be overruled by the registrar.

The 'registrar at startup option' will make Twinkle automatically 
send a REGISTER on startup of the profile.  

## Addressing

When you invite someone to a call you have to enter an address.
A SIP address has the following form:

	sip:<user>@<host-part>

Where 'user' is a user name or a phone number
and 'host-part' is a domain name, FQDN or IP address

The only mandatory part for you to enter is the <user>. Twinkle
will fill in the other parts if you do not provide them.
For the host-part, Twinkle will fill in the value you configured
as your 'domain'.

Currently `sip:` is the only addressing scheme supported by Twinkle.

Michel de Boer [michel@twinklephone.com]

Lubos Dolezel [lubos@dolezel.info]

