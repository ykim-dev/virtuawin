VWPreview v1.1 - Steven Phillips

Description:

This VirtuaWin  module creates a preview of all desktops in either a window or
full screen mode allowing the user to see all their desktops. Left clicking on
a desktop will change to that desktop, right  clicking  anywhere will open the
module's  setup dialog. The Tab,  Shift+Tab and Enter keys can also be used to
change desktops, Esc closes the preview. The module requires VirtuaWin version
4.2 or above.


Installation:

Copy VWPreview.exe into the VirtuaWin modules directory.


Notes:

* The module supports 2 modes, Desktop Preview which uses the whole screen and
  Desktop Select which uses a standard window (size is configurable).
* The module has not been  tested on  multiple  monitor  systems and while the
  author does not  envisage any  particularly  nasty  problems the full screen
  preview is unlikely to work well.
* Desktops  which have not yet been  visited will be rendered  blank using the
  background colour, if a background image is being used the background colour
  is typically  black which can make the preview  display rather  unhelpful. A
  future  enhancement  would be to initialise these desktops to the background
  image instead.


Copyright:

VWPreview is a freely distributed program and is licensed under the GNU
General Public License.

Copyright (c) 2010 VirtuaWin (virtuawin@home.se)


HISTORY

Ver 1.3 - 2010/07/10

* 3025853 Stopped preview window appearing in the desktop previews.
* 3025723 Fixed further preview dialog window issues on Windows XP.
* 3025759 Improved the  quality of the desktop images by using HALFTONE on the
  platforms which support it.
* 3025763 Fixed  keyboard  control  when  preview is opened via the  VirtuaWin
  control menu

Ver 1.2 - 2010/06/21

* Fixed preview dialog window issues on Windows XP.


Ver 1.1 - 2010/06/15

* Added Full Screen 'Update desktop  previews'  option - this option should be
  used with care as it must  continually  cycle  through  all  desktops so the
  computer  will be  constantly  working;  the  screen  will also  flash as it
  updates the preview image for a desktop. Other side effects include changing
  desktop even when Escape is pressed  because this simply  closes the preview
  window and therefore will leave you on a random desktop. If the update speed
  is not right for you then edit the  "fulUdtim#  1000" line in  vwpreview.cfg
  file located in the VirtuaWin Config directory (VirtuaWin -> Setup -> Expert
  -> Explore Config), 1000 = 1sec, save changes then Reload modules (VirtuaWin
  -> Setup ->  Modules  ->  Reload);  if you make  this  number  too small the
  desktop  previews  will be wrong  (missing  windows  or  windows  from other
  desktops  etc),  making  this  number  large  will  reduce  the load on your
  computer.
  
* Added  Window  'Close  after  desktop  change', if disabled  VWPreivew  will
  automatically  update  the  preview  (update  rate can be changed by editing
  "winUdtim# 1000" line - see option above). This make VWPreview act more like
  a very basic desktop pager.


Ver 1.0 - 2010/06/10

* Initial release
