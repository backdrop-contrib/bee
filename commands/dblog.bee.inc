<?php
/**
 * @file
 * Command(s) for displaying database log information.
 */

/**
 * Implements hook_bee_command().
 */
function dblog_bee_command() {
  return array(
    'log' => array(
      'description' => bt('Show database log messages.'),
      'callback' => 'log_bee_callback',
      'group' => 'information',
      'arguments' => array(
        'id' => bt('The ID of a log message to display in detail. Leave blank to list summaries of the most recent log messages.'),
      ),
      'optional_arguments' => array('id'),
      'options' => array(
        'count' => array(
          'description' => bt('The number of log messages to show. Default is 10.'),
          'value' => bt('Number'),
        ),
        'severity' => array(
          'description' => bt('Restrict list to messages of a given severity level.'),
          'value' => bt('Value'),
        ),
        'type' => array(
          'description' => bt('Restrict list to messages of a given type.'),
          'value' => bt('Type'),
        ),
      ),
      'aliases' => array('ws', 'dblog', 'watchdog-show'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee log 3551' => bt("Show details about the log message with ID '3551'."),
        'bee log --count=25 --severity=error --type=php' => bt('Show the 25 latest PHP errors.'),
        'bee log' => bt('Show the latest 10 log messages.'),
      ),
    ),
  );
}

/**
 * Command callback: Show database log messages.
 */
function log_bee_callback($arguments, $options) {
  if (empty($arguments['id'])) {
    // Show multiple log messages.
    return dblog_bee_show_messages($options);
  }
  else {
    // Show one log message.
    $message = db_query('SELECT * FROM {watchdog} WHERE wid = :wid', array(
      ':wid' => $arguments['id'],
    ))->fetchAssoc();

    if (!$message) {
      bee_message(bt('Log message #!id could not be found.', array(
        '!id' => $arguments['id'],
      )), 'error');
      return;
    }

    $severity_levels = watchdog_severity_levels();
    $rows = array(
      array(
        array('value' => bt('Date')),
        array('value' => format_date($message['timestamp'], 'medium')),
      ),
      array(
        array('value' => bt('Type')),
        array('value' => $message['type']),
      ),
      array(
        array('value' => bt('Severity')),
        array('value' => backdrop_ucfirst($severity_levels[$message['severity']])),
      ),
      array(
        array('value' => bt('Message')),
        array('value' => dblog_bee_get_message($message)),
      ),
    );

    echo "\n";
    return array(
      array(
        'type' => 'table',
        'variables' => array(
          'rows' => $rows,
          'delimiter' => '   ',
          'delimiter_left' => ' ',
          'delimiter_right' => ' ',
        ),
      ),
    );
  }
}

/**
 * Display a list of the latest log messages.
 */
function dblog_bee_show_messages($options) {
  $severity_levels = array_map('strtolower', watchdog_severity_levels());

  // Set default value.
  if (!isset($options['count'])) {
    $options['count'] = 10;
  }

  // Get messages.
  $query = db_select('watchdog', 'wd')
    ->fields('wd')
    ->orderBy('wid', 'DESC');
  if (isset($options['type'])) {
    $query->condition('type', $options['type']);
  }
  if (isset($options['severity'])) {
    $severity = backdrop_strtolower($options['severity']);
    $severity = array_search($severity, $severity_levels);
    $query->condition('severity', $severity);
  }
  $messages = $query->range(0, $options['count'])
    ->execute()
    ->fetchAll(PDO::FETCH_ASSOC);

  // Get table rows.
  $rows = array();
  foreach ($messages as $message) {
    $rows[] = array(
      array('value' => $message['wid']),
      array('value' => format_date($message['timestamp'], 'short')),
      array('value' => $message['type']),
      array('value' => backdrop_ucfirst($severity_levels[$message['severity']])),
      array('value' => dblog_bee_get_message($message)),
    );
  }

  // Get table header.
  $header = array(
    array('value' => bt('ID')),
    array('value' => bt('Date')),
    array('value' => bt('Type')),
    array('value' => bt('Severity')),
    array('value' => bt('Message')),
  );

  return array(
    array(
      'type' => 'table',
      'variables' => array(
        'rows' => $rows,
        'header' => $header,
      ),
    ),
  );
}

/**
 * Prepare a log message for output.
 */
function dblog_bee_get_message($data) {
  $variables = $data['variables'];
  $message = $data['message'];

  if (is_string($variables)) {
    $variables = unserialize($variables);
  }
  if (is_array($variables)) {
    $message = bt($message, $variables);
  }

  $message = str_replace("\n", ' ', $message);
  return strip_tags($message);
}
