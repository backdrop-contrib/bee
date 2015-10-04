<?php
// This needs to be set to the path to your Backdrop installation.
$backdrop_path = '/Users/geoff/Sites/backdrop';
require_once($backdrop_path . '/settings.php');
/*define('BACKDROP_ROOT', "$backdrop_path");
require_once BACKDROP_ROOT . '/core/includes/bootstrap.inc';
print BACKDROP_BOOTSTRAP_FULL . "\n";
print backdrop_bootstrap(BACKDROP_BOOTSTRAP_FULL);

print BACKDROP_ROOT;*/

// get DB connection info for PDO object from Backdrop settings.php file.
$info = explode('/', $database);
$host_and_creds = explode('@', $info[2]);
$host = $host_and_creds[1];
$creds = explode(':', $host_and_creds[0]);
$b_user = $creds[0];
$b_pass = $creds[1];

// TODO: need to make DB table map of Backdrop cache tables.
$dbh = new PDO('mysql:host=localhost;dbname=mysql', $b_user, $b_pass);
// Backdrop Database PDO Object.
$bdb = new PDO('mysql:host=localhost;dbname=backdrop', $b_user, $b_pass);

$sql = $dbh->prepare(
  "select table_name from innodb_table_stats
    where database_name = 'backdrop'
    and table_name like '%cache_%'
  "
);
$sql->execute();
$ctables = $sql->fetchAll();

// print b help.
if (count($argv) == 1) {
  print help();
}

if (count($argv) > 1) {
  // Clear all cache
  if ($argv[1] == 'cc') {
    if (count($argv) == 2) {
      $cache_menu = cache_menu();
      print "\nEnter a number to choose which cache to clear.\n";
      foreach ($cache_menu as $key => $c) {
        print "\t$key \t:\t $c\n";
      }
      $handle = fopen ("php://stdin","r");
      $line = fgets($handle);
      $line = trim($line);

      switch($line) {
        case 0:
          print "Canceled\n";
        break;
        case 1:
          cache_clear_all($bdb, $ctables);
        break;
      }
    }
    if (count($argv) == 3) {
      if ($argv[2] == 'all') {
        cache_clear_all($bdb, $ctables);
      }
    }
  }
  if ($argv[1] == 'dl') {
    require_once 'commands/dl.php';
    if (isset($argv[2])) {
      $i = 2;
      while (isset($argv[$i])) {
        dl_project($argv[$i]);
        $i++;
      }
    }
    else {
      print "You must specify a valid project, i.e. 'b dl redirect'.\n";
    }

  }
}

function cache_selection($index) {
  $caches = array(
    0 => 'cancel',
    1 => 'all',
    2 => 'admin-menu',
    3 => 'css-js',
    4 => 'layout',
    5 => 'menu',
    6 => 'page',
    7 => 'theme',
    8 => 'token',
    9 => 'update-data',
  );
  return $caches[$index];
}

function cache_menu() {
  $caches = array(
    0 => 'cancel',
    1 => 'all',
    2 => 'admin-menu',
    3 => 'css-js',
    4 => 'layout',
    5 => 'menu',
    6 => 'page',
    7 => 'theme',
    8 => 'token',
    9 => 'update-data',
  );
  return $caches;
}

function help() {
  print "\n";
  print "\t b commands\n";
  print "\t\tb cc \t// clear cache menu\n";
  print "\t\tb cc all \t// clear all cache\n";
  print "\t\tb dl module \t// download one or more modules\n";
  print "\t that's it;";
  print "\n\n";
}

/**
 * Clear all cache.
 * @param $database
 *   PDO database object to act on
 * @param $tables
 *   tables to truncate
 *
 */
function cache_clear_all($database, $tables) {
  foreach($tables as $c) {
    $cc = $database->prepare(
      "truncate table $c[0]"
    );
    $cc->execute();
  }
  print "All caches cleared.\n";
}
?>