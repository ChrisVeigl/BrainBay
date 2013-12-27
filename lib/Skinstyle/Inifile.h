/* SkinStyle - Win32 Skinning Library 
 * Author: John Roark <jroark@cs.usfca.edu>
 */

/*
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef _INIFILE_H_
#define _INIFILE_H_

class IniFile
{
public:
	IniFile(char *file);
	IniFile() { m_Name = NULL; }
	virtual ~IniFile() { Free(); }
	void Load(char *file);

	void Free()
	{
		if(m_Name != NULL)
			::GlobalFree(m_Name);
	}

	char *ReadString(char * m_Sec, char * m_Ident, char * m_Def);
	BOOL WriteString(char * m_Sec, char * m_Ident, char * m_Val);

	char *m_Name;
};

#endif //_INIFILE_H_
