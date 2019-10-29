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
  $items['user-password'] = array(
    'description' => 'Reset a users password for the user account with the specified name.',
    'bootstrap' => B_BOOTSTRAP_FULL,
    'callback' => 'userpassword_b_callback',
    'aliases' => array('upw'),
    'arguments' => array(
      'username' => array('The username of the account to modify.', 'required' => TRUE),
    ),
    'options' => array(
      'password' => array(
        'description' => 'The new password for the account.',
      ),
    ),
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

/**
 * Sets the password for the account with the given username.
 */
function userpassword_b_callback($arguments, $options) {
  $account = user_load_by_name($arguments['username']);
  if (!is_object($account)) {
    b_set_message(bt('Username !user not found.', array('!user' => $arguments['username'])), 'error');
    return;
  }
  $new_password = '';
  if (isset($options['password'])) {
    $new_password = $options['password'];
  }
  else {
    $new_password = b_prompt('New password', backdrop_random_key(12));
  }
  $account->pass = $new_password;
  user_save($account);
  b_set_message(bt('(Re)set password for !user', array('!user' => $account->name)));
}
