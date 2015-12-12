<?php
/**
 * Print out b status information.
 * @args $path
 *   the path of the Backdrop installation
 */
function status($path) {
  if (file_exists($path . '/settings.php')) {
    print "\033[36mBackdrop CMS Installation detected; wooot; \033[0m\n";
  }
  else {
    print "\033[032m No Backdrop installation found. :( \033[0m\n";
  }
}
?>