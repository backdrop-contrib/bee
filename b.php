<?php
// $path is the root of the backdrop installation.
$path = getcwd();

if (file_exists($path . '/settings.php')) {
  require_once "settings.php";
  // get DB connection info for PDO object from Backdrop settings.php file.
  $info = explode('/', $database);
  $host_and_creds = explode('@', $info[2]);
  $host = $host_and_creds[1];
  $creds = explode(':', $host_and_creds[0]);
  $b_user = $creds[0];
  $b_pass = $creds[1];

  // TODO: need to make DB table map of Backdrop cache tables.
  //$dbh = new PDO("mysql:host=$host;dbname=mysql", $b_user, $b_pass);

  // Backdrop Database PDO Object.
  $bdb = new PDO("mysql:host=$host;dbname=$info[3]", $b_user, $b_pass);
}
else {
  print "\033[31mNo Backdrop installation was found :(.\033[0m\n";
  return 0;
}

// print b help.
if (count($argv) == 1) {
  print help();
}

if (count($argv) > 1) {
  // Clear all cache
  if ($argv[1] == 'cc') {
    // get the backdrop cache tables;
    $p = $bdb->prepare(
        "show tables"
    );
    $p->execute();
    $my_tables = $p->fetchAll();
    $arr_of_tables = array();
    foreach ($my_tables as $t) {
      if (strpos($t[0], 'cache') !== FALSE) {
        $arr_of_tables[] = $t[0];
      }
    }
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
          b_cache_clear_all($bdb, $arr_of_tables);
        break;
        // TODO: build out the rest of the cache_menu cases.
      }
    }
    if (count($argv) == 3) {
      if ($argv[2] == 'all') {
        b_cache_clear_all($bdb, $arr_of_tables);
      }
    }
  }
  if ($argv[1] == 'dl') {
    require_once 'commands/dl.php';
    if (isset($argv[2])) {
      $i = 2;
      while (isset($argv[$i])) {
        dl_project($argv[$i], $path);
        $i++;
      }
    }
    else {
      print "You must specify a valid project, i.e. 'b dl redirect'.\n";
    }
  }
  if ($argv[1] == 'status' || $argv[1] == 'st') {
    require_once "commands/status.php";
    status($path);
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
  echo   "\033[32m \tb commands \n";
  print "\t\tb cc \t\t// clear cache menu\n";
  print "\t\tb cc all \t// clear all cache\n";
  print "\t\tb dl module \t// download one or more modules\n";
  print "\tthat's it; \033[0m";
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
function b_cache_clear_all($database, $tables) {
  foreach($tables as $c) {
    $cc = $database->prepare(
      "truncate table $c"
    );
    $cc->execute();
  }
  print "\033[32mAll caches cleared.\033[0m\n";
}
?>