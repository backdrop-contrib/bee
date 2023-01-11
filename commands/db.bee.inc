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
      'options' => array(
        'extra' => array(
          'description' => bt("Add custom option(s) to the 'mysqldump' command. Do not include leading or trailing spaces and enclose multiple options in double quotes."),
          'value' => bt('Custom mysqldump options'),
        ),
      ),
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

  // Get any extra mysqldump options entered.
  $extra = (!empty($options['extra'])) ? $options['extra'] . ' ' : '';

  // Export and compress the database.
  $export_command = 'mysqldump ';
  $export_command .= db_bee_mysql_options($db_info);
  $export_command .= $extra;
  $export_command .= ' | gzip > ';
  $export_command .= $filename . '.gz';
  exec($export_command, $output, $result);
  // Create debug message with the compiled export command.
  bee_message(bt("The compiled export command is: !export", array(
    '!export' => $export_command,
  )), 'log');

  if ($result === 0) {
    bee_message(bt("The '!database' database was exported to '!filename.gz'.", array(
      '!database' => rawurldecode($db_info['database']),
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
  $import_command .= 'mysql ' . db_bee_mysql_options($db_info);
  if (!$gzip) {
    $import_command .= " < $filename";
  }
  exec($import_command, $output, $result);

  if ($result === 0) {
    bee_message(bt("'!filename' was imported into the '!database' database.", array(
      '!filename' => $filename,
      '!database' => rawurldecode($db_info['database']),
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

  // Get database connection details.
  $connection_details = db_bee_mysql_options($db_info);
  $connection_string = $connection_details['connection'];
  $connection_file = $connection_details['filename'];

  // Open SQL command-line.
  $command = 'mysql ' . $connection_string;
  proc_close(proc_open($command, array(STDIN, STDOUT, STDERR), $pipes));
  // Remove the temp file with username, password and host in.
  bee_delete($connection_file);
}

/**
 * Returns a decoded options string for use with MySQL commands.
 *
 * @param array $db_info
 *   Connection information for the current database. Generally obtained via
 *   Database::getConnectionInfo().
 *
 * @return array
 *   An array of command options that can be appended to 'mysql' or 'mysqldump'.
 */
function db_bee_mysql_options(array $db_info) {
  // Get the username.
  $db_username = rawurldecode($db_info['username']);
  // Get the password.
  $db_password = rawurldecode($db_info['password']);
  // Get the host.
  $db_host = $db_info['host'];
  // Get the temporary directory.
  $temp_dir = bee_get_temp();
  // Create a temporary file for the username, password and host.
  $temp_filename = $temp_dir . '/db_bee.cnf';
  $temp_file = fopen($temp_filename, 'a');
  fwrite($temp_file, "[client]" . PHP_EOL);
  fwrite($temp_file, "user=" . $db_username . PHP_EOL);
  fwrite($temp_file, "password=" . $db_password . PHP_EOL);
  fwrite($temp_file, "host=" . $db_host . PHP_EOL);
  fclose($temp_file);

  // Create an array for the connection string and filename.
  $options = array();
  $options['connection'] = "--defaults-file=$temp_filename";
  $options['connection'] .= ' ' . rawurldecode($db_info['database']);
  $options['filename'] = $temp_filename;

  return $options;
}
