<?php

/**
 * @file
 * Command(s) related to Backdrop users.
 */

/**
 * Implements hook_b_command().
 */
function user_b_command() {
  $items['user-list'] = array(
    'description' => 'List all users.',
    'bootstrap' => B_BOOTSTRAP_FULL,
    'callback' => 'userlist_b_callback',
    'aliases' => array('uls'),
  );

  return $items;
}

/**
 * Callback function for hook_b_command().
 */
function userlist_b_callback($arguments, $options) {
  $rows = array();
  $fields = array('uid', 'name', 'mail', 'login', 'status');
  $rows[] = array(
    'UID',
    'Name',
    'Email',
    'Latest login',
    'Status',
  );
  $result = db_select('users', 'u')
    ->fields('u', $fields)
    ->condition('uid', 0, '>')
    ->execute()
    ->fetchAll(PDO::FETCH_ASSOC);
  foreach ($result as $key => $value) {
    $record = array();
    $record[] = $value['uid'];
    $record[] = $value['name'];
    $record[] = $value['mail'];
    if ($value['login'] > 0) {
      $record[] = format_date($value['login'], 'short');
    }
    else {
      $record[] = 'never';
    }
    if ($value['status'] > 0) {
      $record[] = 'active';
    }
    else {
      $record[] = 'blocked';
    }
    $rows[] = $record;
  }

  return array(
    '#type' => 'table',
    'rows' => $rows,
    'delimiter' => '  :  ',
    'delimiter_left' => ' ',
    'delimiter_right' => ' ',
  );
}
