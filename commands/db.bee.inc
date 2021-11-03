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
        'file' => bt('The SQL file where the exported database will be saved. Leave blank to use the current date/time as the filename.'),
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
    'sql' => array(
      'description' => bt("Open an SQL command-line interface using Backdrop's database credentials."),
      'callback' => 'sql_bee_callback',
      'aliases' => array('sqlc', 'sql-cli', 'db-cli'),
      'bootstrap' => BEE_BOOTSTRAP_DATABASE,
      'examples' => array(
        'bee sql' => bt('Open a command-line for the current database.'),
        'bee sql < backup.sql' => bt('Import an SQL file into the current database.'),
      ),
    ),
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
function db_import_bee_callback($arguments, $options) {
  // Get database info.
  $db_connection = Database::getConnectionInfo();
  $db_info = $db_connection['default'];

  // Get the file for the import.
  $filename = $arguments['file'];

  // Determine if this is a compressed file.
  $gzip = FALSE;
  if (substr($filename, -3) == '.gz') {
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
 * Command callback: Open an SQL command-line interface using Backdrop's
 * database credentials.
 */
function sql_bee_callback($arguments, $options) {
  // Get database info.
  $db_connection = Database::getConnectionInfo();
  $db_info = $db_connection['default'];

  // Open SQL command-line.
  $command = 'mysql --user=' . $db_info['username'] . ' --password=' . $db_info['password'] . ' --host=' . $db_info['host'] . ' ' . $db_info['database'];
  proc_close(proc_open($command, array(STDIN, STDOUT, STDERR), $pipes));
}
