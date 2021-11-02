<?php
/**
 * @file
 * Command(s) for working with Backdrop's database.
 */

/**
 * Implements hook_bee_command().
 */
function db_bee_command() {
  return array(
    'db-export' => array(
      'description' => bt('Export the database as a compressed SQL file.'),
      'callback' => 'db_export_bee_callback',
      'arguments' => array(
        'file' => bt('The file where the exported database will be saved. Leave blank to use the current date/time as the filename.'),
      ),
      'optional_arguments' => array('file'),
      'aliases' => array('dbex', 'db-dump', 'sql-export', 'sql-dump'),
      'bootstrap' => BEE_BOOTSTRAP_DATABASE,
      'examples' => array(
        'bee db-export db.sql' => bt('Export the database to db.sql.gz.'),
        'bee db-export' => bt('Export the database to [DATE_TIME].sql.gz.'),
      ),
    ),
    'db-import' => array(
      'description' => bt('Import an SQL file into the current database.'),
      'callback' => 'db_import_bee_callback',
      'arguments' => array(
        'file' => bt('The SQL file to import into the database. Can be gzipped (i.e. *.sql.gz).'),
      ),
      'aliases' => array('dbim', 'sql-import'),
      'bootstrap' => BEE_BOOTSTRAP_DATABASE,
      'examples' => array(
        'bee db-import backup.sql' => bt('Import backup.sql into the current database.'),
        'bee db-import db.sql.gz' => bt('Extract and import db.sql into the current database.'),
      ),
    ),
    // 'sql-cli' => array(
    //   'description' => bt('Open a SQL command-line interface using Backdrop\'s credentials.'),
    //   'callback' => '_sql_cli_execute',
    //   'aliases' => array('sqlc'),
    //   'examples' => array(
    //     'bee sql-cli < example.sql' => bt('Import sql statements from a file into the current database.'),
    //   ),
    // ),
    // 'sql-drop' => array(
    //   'description' => bt('Drop all tables in a given database.'),
    //   'callback' => '_sql_drop_execute',
    //   'options' => array(
    //     'yes' => array(
    //       'description' => bt('Skip confirmation and proceed.'),
    //     ),
    //     'result-file' => array(
    //       'description' => bt('Save to a file. The file should be relative to Backdrop root.'),
    //     ),
    //   ),
    // ),
  );
}

/**
 * Command callback: Export the database as a compressed SQL file.
 */
function db_export_bee_callback($arguments, $options) {
  global $_bee_backdrop_root;

  // Get database info.
  $db_connection = Database::getConnectionInfo();
  $db_info = $db_connection['default'];

  // Get the filename for the export.
  $filename = isset($arguments['file']) ? $arguments['file'] : date('Ymd_His') . '.sql';

  // Export and compress the database.
  $export_command = 'mysqldump --user=' . $db_info['username'] . ' --password=' . $db_info['password'] . ' --host=' . $db_info['host'] . ' ' . $db_info['database'] . ' | gzip > ' . $filename . '.gz';
  exec($export_command, $output, $result);

  if ($result === 0) {
    bee_message(bt("The '!database' database was exported to '!filename.gz'.", array(
      '!database' => $db_info['database'],
      '!filename' => $_bee_backdrop_root . '/' . $filename,
    )), 'success');
  }
}

/**
 * Command callback: Import an SQL file into the current database.
 */
function db_import_bee_callback() {
  // Get database info.
  $db_connection = Database::getConnectionInfo();
  $db_info = $db_connection['default'];

  // Get the file for the import.
  $filename = $arguments['file'];

  // Determine if this is a compressed file.
  $gzip = FALSE;
  if (substr($file, -3) == '.gz') {
    $gzip = TRUE;
  }

  // Import the database.
  $import_command = '';
  if ($gzip) {
    $import_command .= "gunzip -c $filename | ";
  }
  $import_command .= 'mysql --user=' . $db_info['username'] . ' --password=' . $db_info['password'] . ' --host=' . $db_info['host'] . ' ' . $db_info['database'];
  if (!$gzip) {
    $import_command .= " < $filename";
  }
  exec($import_command, $output, $result);

  if ($result === 0) {
    bee_message(bt("'!filename' was imported into the '!database' database.", array(
      '!filename' => $filename,
      '!database' => $db_info['database'],
    )), 'success');
  }
}

/**
 * Opens a SQL command-line interface using Backdrop's credentials.
 */
function _sql_cli_execute() {
  global $_bee_backdrop_root;
  if (!empty($_bee_backdrop_root)) {
    if (class_exists('Database')) {
      $db_connection = Database::getConnectionInfo();
      $db = array_shift($db_connection);
      $query = "mysql --user={$db['username']} --password={$db['password']} --host={$db['host']} {$db['database']}";
      exec($query);
    }
  }
  else {
    // No Backdrop root found.
    bee_message(bt("No Backdrop installation found. Run this command again from within a Backdrop installation, or use the '--root' global option."), 'warning');
  }
}

/**
 * Drops all tables in a given database..
 */
function _sql_drop_execute() {
  global $_bee_backdrop_root;
  if (!empty($_bee_backdrop_root)) {
    if (class_exists('Database')) {
      $db_connection = Database::getConnectionInfo();
      $db = array_shift($db_connection);
      if (!bee_confirm(bt('Do you really want to drop all tables?'))) {
        return;
      }
      $connection = mysqli_connect($db['host'], $db['username'], $db['password']);
      // Drop Database
      $query = "DROP DATABASE `{$db['database']}`";
      if ($connection->query($query) === TRUE) {
        // Re-create the database.
        $query = "CREATE DATABASE `{$db['database']}`";
        $connection->query($query);
      } else {
        bee_message(bt('Unable to drop database ' . $connection->error, array(
          '!database' => $db['database'],
        )), 'error');
      }
      $connection->close();
    }
  }
  else {
    // No Backdrop root found.
    bee_message(bt("No Backdrop installation found. Run this command again from within a Backdrop installation, or use the '--root' global option."), 'warning');
  }
}
