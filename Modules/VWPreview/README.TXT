VWPreview v1.5 - Steven Phillips

Description:

This VirtuaWin  module creates a preview of all desktops in either a window or
full screen mode allowing the user to see all their desktops. Left clicking on
a desktop will change to that desktop, right  clicking  anywhere will open the
module's  setup dialog. The Tab,  Shift+Tab and Enter keys can also be used to
change desktops, Esc closes the preview. The module requires VirtuaWin version
4.2 or above.

The module can also provide basic desktop change transition effects, for these
version 4.3 of VirtuaWin or above is required.


Installation:

Copy VWPreview.exe into the VirtuaWin modules directory.


Notes:

* The module supports 2 modes, Desktop Preview which uses the whole screen and
  Desktop Select which uses a standard window (size is configurable).
* The  module  can also  provide  desktop  change  transition  effect if using
  VirtuaWin v4.3 or later.
* The module has not been  tested on  multiple  monitor  systems and while the
  author does not  envisage any  particularly  nasty  problems the full screen
  preview or transition effects are unlikely to work well.
* Desktops  which have not yet been  visited  will be rendered  largely  blank
  using  the  background  colour,  if a  background  image  is being  used the
  background  colour is  typically  black  which can make the preview  display
  rather unhelpful. A future enhancement would be to initialise these desktops
  to the background image instead.
* There are now two time values in the vwpreview.cfg file:
    fulUdTm1# - Initial update time (in ms) used to  update all previews,
                default is 2sec (i.e. 2000)  
    fulUdTm2# - Continued update time (in ms), default is 10sec.
* Found that update does not work well when  Windows  Themes are used  because
  the screen has not been  properly  updated. To avoid the  problem a pause is
  needed to allow the  screen to be  redrawn,  set option  'fulUdPas#'  in the
  vwpreview.cfg file to say 500 for a half second pause. 


Copyright:

VWPreview is a freely distributed program and is licensed under the GNU
General Public License.

Copyright (c) 2010-2011 VirtuaWin (virtuawin@home.se)


HISTORY

Ver 1.5 - 2011/02/20

* 3152494 Merged in patch submitted by rolfbrocke  adding slide-on  and center
  zoom transition effects.
* Added new Effect duration setting. 

Ver 1.4 - 2010/09/12

* 3064557  Implemented a new  Continuous  update option which, when  disabled,
  stops the update once all desktops  have been updated. Also changed to using
  two update times, an initial time for a quick  update and a second timer for
  slower continued update.
* 3064553 Implemented some basic desktop change transition effects, bit slow
  as it doesn't use DirectX
* 3025866 Fixed VirtuaWin window focus problem.
* 3025763 Fixed key input issue.

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
