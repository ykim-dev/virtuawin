//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, 
//  USA.
//

#include <windows.h>
#include "ListStructures.h"

#ifndef _DISKROUTINES_H_
#define _DISKROUTINES_H_

void loadFilePaths();
void writeDisabledList(int* theNOfModules, moduleType* theModList);
int loadDisabledModules(disModules* theDisList);
int loadStickyList(stickyType* theStickyList);
void saveStickyWindows(int* theNOfWin, windowType* theWinList);
void saveDesktopState(int* theNOfWin, windowType* theWinList);
void saveDesktopConfiguration(int* theNOfWin, windowType* theWinList);
int loadAssignedList(assignedType* theAssignList);
int loadUserList(userType* theUserList);

#endif

/*
 * $Log$
 */
