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
      'description' => bt('Export the database as a compressed SQL file. This uses the --no-tablespaces option by default.'),
      'callback' => 'db_export_bee_callback',
      'arguments' => array(
        'file' => bt('The SQL file where the exported database will be saved. Leave blank to use the current date/time as the filename.'),
      ),
      'optional_arguments' => array('file'),
      'options' => array(
        'extra' => array(
          'description' => bt("Add custom option(s) to the 'mysqldump' command. Do not include leading or trailing spaces and enclose multiple options in double quotes. '--no-tablespaces' option is not used unless you add it."),
          'value' => bt('Custom mysqldump options'),
        ),
      ),
      'aliases' => array('dbex', 'db-dump', 'sql-export', 'sql-dump'),
      'bootstrap' => BEE_BOOTSTRAP_DATABASE,
      'examples' => array(
        'bee db-export db.sql' => bt('Export the database to db.sql.gz.'),
        'bee db-export' => bt('Export the database to [DATE_TIME].sql.gz.'),
        'bee db-export --extra=" " db.sql' => bt("Export the database to db.sql.gz without using the '--no-tablespaces' option."),
        'bee db-export --extra="--no-data --no-tablespaces" db.sql' => bt("Export the database without data, and using the '--no-tablespaces' option, to db.sql.gz."),
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
 *
 * Now includes '--no-tablespaces' by default. Many hosts need this for
 * mysqldump to work as the PROCESS permission can't be granted to the Backdrop
 * database user.
 * @see https://anothercoffee.net/how-to-fix-the-mysqldump-access-denied-process-privilege-error/
 */
function db_export_bee_callback($arguments, $options) {
  global $_bee_backdrop_root;

  // Get database info.
  $db_connection = Database::getConnectionInfo();
  $db_info = $db_connection['default'];

  // Get database connection details.
  $connection_details = db_bee_mysql_options($db_info);
  $connection_string = $connection_details['connection'];
  $connection_file = $connection_details['filename'];

  // Get the filename for the export.
  $filename = isset($arguments['file']) ? $arguments['file'] : date('Ymd_His') . '.sql';

  // Check if any extra mysqldump options have been entered.
  if (!empty($options['extra'])) {
    // Get extra mysqldump options. If user has added the extra option, then we
    // don't include '--no-tablespaces' by default to allow the user to
    // override this setting through ommission.
    $extra = $options['extra'] . ' ';
  }
  else {
    // Include '--no-tablespaces' by default.
    $extra = '--no-tablespaces ';
  }

  // Export and compress the database.
  $export_command = 'mysqldump ';
  $export_command .= $connection_string;
  $export_command .= ' ';
  $export_command .= $extra;
  $export_command .= ' | gzip > ';
  $export_command .= $filename . '.gz';
  exec($export_command, $output, $result);
  // Remove the temporary mysql options file.
  bee_delete($connection_file);

  // Create debug message with the compiled export command.
  bee_message(bt("The compiled export command is: !export", array(
    '!export' => $export_command,
  )), 'log');

  if ($result === 0) {
    // Compile the correct filename based on whether path is absolute or
    // relative.
    if (substr($filename, 0, 1) != '/') {
      // Filename is relative path.
      $filename = $_bee_backdrop_root . '/' . $filename;
    }
    bee_message(bt("The '!database' database was exported to '!filename.gz'.", array(
      '!database' => rawurldecode($db_info['database']),
      '!filename' => $filename,
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

  // Get database connection details.
  $connection_details = db_bee_mysql_options($db_info);
  $connection_string = $connection_details['connection'];
  $connection_file = $connection_details['filename'];

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
  $import_command .= 'mysql ' . $connection_string;
  if (!$gzip) {
    $import_command .= " < $filename";
  }
  exec($import_command, $output, $result);
  // Remove the temporary mysql options file.
  bee_delete($connection_file);

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
  // Remove the temporary mysql options file.
  bee_delete($connection_file);
}

/**
 * Puts the decoded connection options into a temporary option file and returns
 * an options string pointing to the file for use with MySQL commands and the
 * filename separately, so it can be deleted after use.
 *
 * @param array $db_info
 *   Connection information for the current database. Generally obtained via
 *   Database::getConnectionInfo().
 *
 * @return array
 *   An array of the option string that can be appended to 'mysql' or
 *   'mysqldump', together with the filename of the temporary options file.
 */
function db_bee_mysql_options(array $db_info) {
  // Prepare the credentials for the .cnf file.
  $temp_file_contents = "[client]" . PHP_EOL
    . "user=" . rawurldecode($db_info['username']) . PHP_EOL
    . "password='" . rawurldecode($db_info['password']) . "'" . PHP_EOL
    . "host=" . $db_info['host'] . PHP_EOL;

  // Get the temporary directory.
  $temp_dir = bee_get_temp();
  // Create a temporary file for the username, password and host.
  $temp_filename = $temp_dir . '/db_bee.cnf';
  $temp_file = fopen($temp_filename, 'a');
  fwrite($temp_file, $temp_file_contents);
  fclose($temp_file);

  // Create an array for the connection string and filename.
  $options = array();
  $options['connection'] = "--defaults-file=$temp_filename";
  $options['connection'] .= ' ' . rawurldecode($db_info['database']);
  $options['filename'] = $temp_filename;

  return $options;
}
