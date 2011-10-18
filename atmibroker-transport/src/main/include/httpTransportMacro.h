/*
 * JBoss, Home of Professional Open Source
 * Copyright 2011, Red Hat, Inc., and others contributors as indicated
 * by the @authors tag. All rights reserved.
 * See the copyright.txt in the distribution for a
 * full listing of individual contributors.
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License, v. 2.1.
 * This program is distributed in the hope that it will be useful, but WITHOUT A
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 * You should have received a copy of the GNU Lesser General Public License,
 * v.2.1 along with this distribution; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */
/* Export/Include macros for Win32 compilation */
#ifndef _HTTPTRANSPORTMACRO_H_
#define _HTTPTRANSPORTMACRO_H_

/* Only do defines if we're compiling on Win32 */
#ifdef WIN32

#ifdef HTTPTRANSPORT_DLL
#define HTTPTRANSPORT_DLL __declspec(dllexport)
#else
#define HTTPTRANSPORT_DLL __declspec(dllimport)
#endif

#else /* Non-win32 platform. Macros need to pre-process away */

/* quickstarts */

#define HTTPTRANSPORT_DLL

#endif

#endif // _HTTPTRANSPORTMACRO_H_