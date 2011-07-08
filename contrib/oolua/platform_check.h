///////////////////////////////////////////////////////////////////////////////
///  @file platform_check.h
///  Preforms a check of platform defines and defines a macro
///  @remarks
///  Information available via http://predef.sourceforge.net/preos.html
///  @author Liam Devine
///  @email
///  See http://www.liamdevine.co.uk for contact details.
///  @licence
///  See licence.txt for more details. \n 
///////////////////////////////////////////////////////////////////////////////

#ifndef PLATFORM_CHECK_H_
#	define PLATFORM_CHECK_H_

#	ifndef PLATFORM_CHECKED
#		define PLATFORM_CHECKED
#		if (defined(__CYGWIN__))
//#			error Cygwin is currently not supported
#			define UNIX_BUILD			1
#		else
			/// windows 
#			if (defined(__WIN32__) || defined(_WIN32) || defined(WIN32))
#				define WINDOWS_BUILD	1
			/// os2
#			elif (defined(__OS2__) || defined(_OS2)  || defined(OS2) || defined(Macintosh) || \
				defined(macintosh) || defined(__MACOSX__) || defined(__APPLE__))
#				define MAC_BUILD		1
			/// nix
#			elif (defined(unix) || defined(_unix) || defined(__unix) || defined(__unix__) || \
				defined(linux) || defined(__linux))
#				define UNIX_BUILD		1

#			endif///! CYGWIN
#		endif///CYGWIN

#	endif///PLATFORM_CHECKED
	
#endif ///PLATFORM_CHECK_H_

