<?php
/**
 * Implementation of hook_b_command().
 */
 
function dblog_b_command() {
  $items = array();
  $items['watchdog-show'] = array(
    'bootstrap' => B_BOOTSTRAP_FULL,
    'description' => 'Show dblog messages.',
    'arguments' => array(
      'wid' => 'Optional id of a watchdog message to show in detail. If not provided, a listing of most recent 10 messages will be displayed. Alternatively if a string is provided, watchdog messages will be filtered by it.',
    ),
    'options' => array(
      'count' => 'The number of messages to show. Defaults to 10.',
      'severity' => 'Restrict to messages of a given severity level.',
      'type' => 'Restrict to messages of a given type.',
      'tail' => 'Continuously show new watchdog messages until interrupted.',
      'sleep-delay' => 'To be used in conjunction with --tail. This is the number of seconds to wait between each poll to the database. Delay is 1 second by default.',
    ),
    'callback' => 'dblog_command_show',
    'aliases' => array('wd-show', 'ws'),
  );
  return $items;
}

function dblog_messages(){
  
  
}

function dblog_command_show($arguments, $options) {
  if(isset($arguments['wid'])) {
    return dblog_show_one($arguments['wid']);
  }
  
  return dblog_show_many($options);
}

/**
 * Print a table of watchdog messages.
 *
 * @param $filter
 *   String to filter the message's text by.
 */
function dblog_show_many($options) {
  if(!isset($options['count'])) {
    $options['count'] = 10;
  }    
  $query = db_select('watchdog', 'wd')->extend('PagerDefault')
    ->fields('wd');

  if(isset($options['type'])) {
    $query->condition('type', $options['type']);
  }

  if(isset($options['severity'])) {
    $severities = dblog_get_severities();
    if($key = array_search($options['severity'], $severities)){
      $query->condition('severity', $key);
    }
    else{
      b_set_message(bt('There is no Severity !name', array('!name' => $options['severity'])), 'warning');
    }
  }
  
  $messages =  $query->limit($options['count'])->execute()->fetchAll();
  
  $rows = array();
  foreach($messages as $message) {
    $rows[] = array(
      $message->wid,
      format_date($message->timestamp, 'custom', 'd/M H:i'),
      $message->type,
      dblog_colored_severities_output($message->severity),
      dblog_return_message($message),
    );
  }
  
  $header = array(
    'ID',
    'Date',
    'Type',
    'Severity',
    'Message',
  );
  
  return array(
    '#type' => 'table',
    'rows' => $rows,
    'header' => $header,
  );

}

/**
 * Return message with already replaced variables and made plain.
 */
function dblog_return_message($message) {
  $variables = $message->variables;
  if (is_string($variables)) {
    $variables = unserialize($variables);
  }
  if (is_array($variables)) {
    $message->message = bt($message->message, $variables);
  }
  $message->message = str_replace("\n", " ", $message->message);
  return strip_tags($message->message);
}

/**
 * Return array of human readable severities.
 */
function dblog_get_severities($severity = FALSE){
  $severities = array(
    WATCHDOG_DEBUG => 'debug',
    WATCHDOG_INFO => 'info',
    WATCHDOG_NOTICE => 'notice',
    WATCHDOG_WARNING => 'warning',
    WATCHDOG_ERROR => 'error',
    WATCHDOG_CRITICAL => 'critical',
    WATCHDOG_ALERT => 'alert',
    WATCHDOG_EMERGENCY => 'emergency',
  );
  
  if($severity){
    return $severities[$severity];
  }
  return $severities;
}

function dblog_colored_severities_output($severity){
  $title = dblog_get_severities($severity);
  $color = 'cyan';
  switch($title){
    case 'debug':
    case 'error':
    case 'critical':
    case 'alert':
    case 'emergency':
      $color = 'bold_red';
      break;
    case 'notice':
    case 'info':
      $color = 'green';
      break;
    case 'warning':
      $color = 'yellow';
      break;
  }
  
  return array(
    'data' => $title,
    '#color' => $color,
  );
}

/**
 * Print a watchdog message.
 *
 * @param $wid
 *    The id of the message to show.
 */
function dblog_show_one($wid) {
  $message = db_select('watchdog', 'wd')
    ->fields('wd')
    ->condition('wid', $wid)
    ->execute()
    ->fetchObject();
    
  if(!$message) {
    b_set_message(bt('Resent log message #!wid not found.', array('!wid' => $wid)), 'error');
    return FALSE;
  }
  
  $rows = array();
  
  $rows[] = array(
    'ID',
    $message->wid,
  );

  $rows[] = array(
    'Date',
    format_date($message->timestamp, 'custom', 'd/M H:i'),
  );

  $rows[] = array(
    'Type',
    $message->type,
  );
  
  $rows[] = array(
    'Severity',
    dblog_colored_severities_output($message->severity),
  );

  $rows[] = array(
    'Message',
    dblog_return_message($message),
  );
  
  return array(
    '#type' => 'table',
    'rows' => $rows,
    'delimiter' => ' : ',
    'delimiter_left' => ' ',
    'delimiter_right' => ' ',
  );
}
