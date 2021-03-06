/*
 * tnmWinInit.c --
 *
 *	This file contains the Windows specific entry point.
 *
 * Copyright (c) 1996-1997 University of Twente.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

#include "tnmInt.h"
#include "tnmPort.h"

#include <arpa/nameser.h>
#include <resolv.h>

#if defined(__WIN32__)
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#   undef WIN32_LEAN_AND_MEAN

/*
 * VC++ has an alternate entry point called DllMain, so we need to rename
 * our entry point.
 */

#   if defined(_MSC_VER)
#	define EXPORT(a,b) __declspec(dllexport) a b
#	define DllEntryPoint DllMain
#   else
#	if defined(__BORLANDC__)
#	    define EXPORT(a,b) a _export b
#	else
#	    define EXPORT(a,b) a b
#	endif
#   endif
#else
#   define EXPORT(a,b) a b
#endif

/*
 * Forward declarations for procedures defined later in this file:
 */

static void
FixPath				_ANSI_ARGS_((char *path));

static char*
GetRegValue			_ANSI_ARGS_((char *path, char *attribute));

EXPORT(int,Tnm_Init)		_ANSI_ARGS_((Tcl_Interp *interp));

EXPORT(int,Tnm_SafeInit)	_ANSI_ARGS_((Tcl_Interp *interp));


/*
 *----------------------------------------------------------------------
 *
 * DllEntryPoint --
 *
 *	This wrapper function is used by Windows to invoke the
 *	initialization code for the DLL.  If we are compiling
 *	with Visual C++, this routine will be renamed to DllMain.
 *	routine.
 *
 * Results:
 *	Returns TRUE;
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

#ifdef __WIN32__
BOOL APIENTRY
DllEntryPoint(hInst, reason, reserved)
    HINSTANCE hInst;		/* Library instance handle. */
    DWORD reason;		/* Reason this function is being called. */
    LPVOID reserved;		/* Not used. */
{
    return TRUE;
}
#endif

/*
 *----------------------------------------------------------------------
 *
 * FixPath --
 *
 *	This procedure converts a Windows path into the Tcl 
 *	representation by converting "\\" to "/".
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The path is modified.
 *
 *----------------------------------------------------------------------
 */

static void
FixPath(path)
    char *path;
{
    if (path) {
	for (; *path; path++) {
	    if (*path == '\\') {
		*path = '/';
	    }
	}
    }
}

/*
 *----------------------------------------------------------------------
 *
 * GetRegValue --
 *
 *	This procedure retrieves a value from the Windows registry.
 *
 * Results:
 *	A pointer to the string containing the value or NULL if the
 *	value can't be obtained. The string points to static memory
 *	and is overwritten by every call to this procedure.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static char *
GetRegValue(path, attribute)
    char *path;
    char *attribute;
{
    int code;
    HKEY key;
    DWORD size;
    static char *value = NULL;

    if (value) {
	ckfree(value);
	value = NULL;
    }

    code = RegOpenKeyEx(HKEY_LOCAL_MACHINE, path, 0, KEY_READ, &key);
    if (code != ERROR_SUCCESS) {
	return NULL;
    }

    code = RegQueryValueEx(key, attribute, NULL, NULL, NULL, &size);
    if (code != ERROR_SUCCESS) {
	return NULL;
    }

    value = ckalloc(size);
    code = RegQueryValueEx(key, attribute, NULL, NULL, value, &size);
    if (code != ERROR_SUCCESS) {
	ckfree(value);
	value = NULL;
    }

    return value;
}

/*
 *----------------------------------------------------------------------
 *
 * TnmInitPath --
 *
 *	This procedure is called to determine the installation path
 *	of the Tnm extension on this system. It uses the Windows
 *	registry to retrieve the the toplevel Tnm directory name.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Initializes the global Tcl variable tnm(library).
 *
 *----------------------------------------------------------------------
 */

void
TnmInitPath(interp)
    Tcl_Interp *interp;
{
    char *p, *path, *tclVersion, *tkVersion, *tclRoot;
    Tcl_DString ds;
    int i;

    char *candidates[] = {
	"Software\\Scriptics\\Tcl",
	"Software\\Sun\\Tcl",
	NULL
    };

    Tcl_DStringInit(&ds);

    path = GetRegValue("Software\\Scotty\\Tnm\\" TNM_VERSION, "Library");
    if (! path || *path == '\0') {
	path = getenv("TNM_LIBRARY");
        }
    if (! path || *path == '\0') {
	if ( TCL_OK == Tcl_Eval(interp, "file normalize [file join [file dir [info nameofexecutable]] .. lib tnm]") ) {
	    path = Tcl_GetStringResult(interp);
            Tcl_DStringAppend(&ds, path, -1);
            Tcl_DStringAppend(&ds, TNM_VERSION, -1);
	    FixPath(Tcl_DStringValue(&ds));
	    if (access(Tcl_DStringValue(&ds), R_OK) == 0) {
                path = Tcl_DStringValue(&ds);
            }
        }
    }
    if (! path || *path == '\0') {
        path = TNMLIB;
    }
    FixPath(path);
    Tcl_SetVar2(interp, "tnm", "library", path, TCL_GLOBAL_ONLY);
    Tcl_DStringFree(&ds);

    /*
     * Also initialize the tkined(library) variable which is used
     * by tnmIned.c to locate Tkined specific files. This should
     * be removed once Tkined gets integrated into Tnm.
     */

    path = GetRegValue("Software\\Scotty\\Tkined\\" TKI_VERSION, "Library");
    if (! path || *path == '\0') {
	path = getenv("TKINED_LIBRARY");
	}
    if (! path || *path == '\0') {
        if ( TCL_OK == Tcl_Eval(interp, "file normalize [file join [file dir [info nameofexecutable]] .. lib tkined]") ) {
            path = Tcl_GetStringResult(interp);
            Tcl_DStringAppend(&ds, path, -1);
            Tcl_DStringAppend(&ds, TKI_VERSION, -1);
	    FixPath(Tcl_DStringValue(&ds));
	    if (access(Tcl_DStringValue(&ds), R_OK) == 0) {
                path = Tcl_DStringValue(&ds);
            }
        }
    }
    if (! path || *path == '\0') {
	path = TKINEDLIB;
    }
    FixPath(path);
    Tcl_SetVar2(interp, "tkined", "library", path, TCL_GLOBAL_ONLY);
    Tcl_DStringFree(&ds);

    /*
     * Locate tclsh and wish so that we can start additional
     * scripts with the correct path name.
     */

    tclVersion = Tcl_GetVar(interp, "tcl_version", TCL_GLOBAL_ONLY);
    tkVersion = Tcl_GetVar(interp, "tk_version", TCL_GLOBAL_ONLY);

    if (tclVersion) {
	for (i = 0, tclRoot = NULL; candidates[i] && ! tclRoot; i++) {
	    Tcl_DStringAppend(&ds, candidates[i], -1);
	    Tcl_DStringAppend(&ds, "\\", -1);
	    Tcl_DStringAppend(&ds, tclVersion, -1);
	    tclRoot = GetRegValue(Tcl_DStringValue(&ds), "Root");
	    if (! tclRoot) {
	      tclRoot = GetRegValue(Tcl_DStringValue(&ds), "");
            } else if (*tclRoot == '\0') {
                tclRoot = NULL;
	    }
	    Tcl_DStringFree(&ds);
	}
    }

    if (! tclRoot || *tclRoot == '\0') {
        /* Probably easier to do this at Tcl level */
        if ( TCL_OK == Tcl_Eval(interp, "file normalize [file join [file dir [info nameofexecutable]] .. ]") ) {
            tclRoot = Tcl_GetStringResult(interp);
        }
        if (! tclRoot || *tclRoot == '\0') {
	    /*
	     * no install path found in registry
	     * *shrug*, use a default hardcoded install
	     * path as last resort
	     */
	    tclRoot = "C:\\Tcl";
	}
    }

    if (tclVersion && tclRoot) {
	Tcl_DStringAppend(&ds, tclRoot, -1);
	Tcl_DStringAppend(&ds, "\\bin\\tclsh", -1);
	for (p = tclVersion; *p; p++) {
	    if (isdigit(*p)) Tcl_DStringAppend(&ds, p, 1);
	}
	Tcl_DStringAppend(&ds, ".exe", -1);
	FixPath(Tcl_DStringValue(&ds));
	if (access(Tcl_DStringValue(&ds), R_OK | X_OK) == 0) {
	    Tcl_SetVar2(interp, "tnm", "tclsh",
			Tcl_DStringValue(&ds), TCL_GLOBAL_ONLY);
	}
	Tcl_DStringFree(&ds);
    }

    if (tkVersion && tclRoot) {
	Tcl_DStringAppend(&ds, tclRoot, -1);
	Tcl_DStringAppend(&ds, "\\bin\\wish", -1);
	for (p = tkVersion; *p; p++) {
	    if (isdigit(*p)) Tcl_DStringAppend(&ds, p, 1);
	}
	Tcl_DStringAppend(&ds, ".exe", -1);
	FixPath(Tcl_DStringValue(&ds));
	if (access(Tcl_DStringValue(&ds), R_OK | X_OK) == 0) {
	    Tcl_SetVar2(interp, "tnm", "wish",
			Tcl_DStringValue(&ds), TCL_GLOBAL_ONLY);
	}
	Tcl_DStringFree(&ds);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * TnmInitDns --
 *
 *	This procedure is called to initialize the DNS resolver and to
 *	save the domain name in the global tnm(domain) Tcl variable.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

void
TnmInitDns(interp)
    Tcl_Interp *interp;
{
    char domain[MAXDNAME], *p;
    char *nameServer = NULL, *domainName = NULL;
    int i,j;

    char *paths[] = {
	"System\\CurrentControlSet\\Services\\TcpIP\\Parameters",
	"System\\CurrentControlSet\\Services\\Tcpip\\Parameters",
	"System\\CurrentControlSet\\Services\\VxD\\MSTCP",
	NULL
    };

    char *entries[] = {
	"NameServer",
	"DhcpNameServer",
	NULL
	};

    char *default_dns = "127.0.0.1";

    res_init();
    _res.options |= RES_RECURSE | RES_DNSRCH | RES_DEFNAMES | RES_AAONLY;
    
    /*
     * Get the list of name servers from the Windows registry and set
     * the list of default DNS servers. Note, this should be done
     * automatically by the resolver but it is obviously not on
     * Windows machines.
     */

    for (i = 0; paths[i] && ! nameServer; i++) {
	for (j = 0; entries[j] && ! nameServer; j++) {
	    nameServer = GetRegValue(paths[i], entries[j]);
	    if (nameServer) {
		int n = 0;
		char *s = NULL;
		if (*nameServer == '\0') {
		nameServer = NULL;
		continue;
		}
		s = strtok(nameServer, ", ");
		while (s && n < MAXNS) {
		    TnmSetIPAddress((Tcl_Interp *) NULL,
				    s, &_res.nsaddr_list[n]);
		    s = strtok(NULL, ", ");
		    n++;
		}
		_res.nscount = n;
	    }
	}
    }

    if (! nameServer || (nameServer && *nameServer == '\0')) {
	TnmSetIPAddress((Tcl_Interp *) NULL, 
			default_dns, &_res.nsaddr_list[0]);
	_res.nscount = 1;
        nameServer = default_dns;
    }
    Tcl_SetVar2(interp, "tnm", "dns", nameServer, TCL_GLOBAL_ONLY);

    /* Get the domain name from the Windows registry and set the
     * default domain name for the resolver, if not done
     * automatically.
     */

    for (i = 0; paths[i] && ! domainName; i++) {
	domainName = GetRegValue(paths[i], "Domain");
        if (domainName && *domainName == '\0') {
            domainName = NULL;
        }
    }

    if (! _res.defdname[0] && domainName && strlen(domainName) < MAXDNAME) {
	strcpy(_res.defdname, domainName);
    }

    /*
     * Remove the any trailing dots or white spaces and save the
     * result in tnm(domain).
     */

    strcpy(domain, _res.defdname);
    p = domain + strlen(domain) - 1;
    while ((*p == '.' || isspace(*p)) && p > domain) {
	*p-- = '\0';
    }
    Tcl_SetVar2(interp, "tnm", "domain", domain, TCL_GLOBAL_ONLY);
}

/*
 *----------------------------------------------------------------------
 *
 * Tnm_Init --
 *
 *	This procedure is the Windows entry point for trusted Tcl
 *	interpreters.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	Tcl variables are created.
 *
 *----------------------------------------------------------------------
 */

EXPORT(int,Tnm_Init)(interp)
    Tcl_Interp *interp;
{
    return TnmInit(interp, 0);
}

/*
 *----------------------------------------------------------------------
 *
 * Tnm_SafeInit --
 *
 *	This procedure is the Windows entry point for safe Tcl
 *	interpreters.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	Tcl variables are created.
 *
 *----------------------------------------------------------------------
 */

EXPORT(int,Tnm_SafeInit)(interp)
    Tcl_Interp *interp;
{
    return TnmInit(interp, 1);
}
