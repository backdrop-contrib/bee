<?php
/**
 * @file
 * Command(s) for running SQL queries via db_query().
 */

/**
 * Implements hook_bee_command().
 */
function dbq_bee_command() {
  return array(
    'db-query' => array(
      'description' => bt('Execute a query using db_query().'),
      'callback' => 'dbq_bee_callback',
      'group' => 'advanced',
      'arguments' => array(
        'code' => bt("An SQL query as passed to db_query() (no placeholders)."),
      ),
      'aliases' => array('dbq'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee db-query "SELECT * FROM {users} WHERE uid = 1"' => bt('Browse user record. Note that table prefixes are honored.'),
        'bee db-query "UPDATE {users} SET mail = \'me@example.com\' WHERE uid = 1"' => bt('Update a user\'s email address.'),
        'bee db-query "DELETE FROM {users} WHERE uid = 2"' => bt('Delete a user record from the db.'),
        'bee db-query "SELECT name, mail FROM {users} WHERE uid > 0 LIMIT 2"' => bt('Show the name and email for the first 2 users.'),
      ),
    ),
  );
}

/**
 * Command callback: Pass to db_query() after bootstrapping Backdrop.
 *
 * @see sql.drush.inc
 */
function dbq_bee_callback($arguments, $options) {
  try {
    $result = db_query($arguments['code'], array());
    $output = $result->fetchAll(PDO::FETCH_ASSOC);
    if (!empty($output[0])) {
      // Display the output as CSV rows.
      foreach ($output as $row) {
        if (!empty($row)) {
          $f = fopen('php://memory', 'r+');
          if (fputcsv($f, $row) === FALSE) {
              continue;
          }
          rewind($f);
          $csv_line = stream_get_contents($f);
          $text = rtrim($csv_line);
          bee_message($text);
        }
      }
    }
  }
  catch (PDOException $e) {
    bee_message($e->getMessage(), 'error');
  }
}
