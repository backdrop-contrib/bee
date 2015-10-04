<?php
// This needs to be set to the path to your Backdrop installation.
$settings_path = '/Users/geoff/Sites/backdrop/settings.php';
require_once($settings_path);

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
      print "\n\n" . cache_selection($line) . "\n";
      if ($line == 1) {
        cache_clear_all($bdb, $ctables);
      }
    }
    if (count($argv) == 3) {
      if ($argv[2] == 'all') {
        cache_clear_all($bdb, $ctables);
      }
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
  print "\t\tb cc cache \t// clear menu\n";
  print "\t\tb cc all \t// clear all cache\n";
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