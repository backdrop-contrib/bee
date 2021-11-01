<?php
/**
 * @file
 * Command(s) for working with user accounts.
 */

/**
 * Implements hook_bee_command().
 */
function user_bee_command() {
  return array(
    'users' => array(
      'description' => bt('List all user accounts.'),
      'callback' => 'users_bee_callback',
      'aliases' => array('uls', 'user-list'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee users' => bt('Display a list of all user accounts for the current site.'),
      ),
    ),
    'user-password' => array(
      'description' => bt('Reset the login password for a given user.'),
      'callback' => 'user_password_bee_callback',
      'arguments' => array(
        'username' => bt('The username of the user account to reset the password for.'),
        'password' => bt('The new password to use for the given user account. Leave blank to generate a random password.'),
      ),
      'optional_arguments' => array('password'),
      'aliases' => array('upw', 'upwd'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee user-password admin P@55w0rd' => bt("Give the 'admin' user account an insecure password (not recommended)."),
        'bee user-password drop "Too hard to guess, too long to brute-force."' => bt("Give the 'drop' user account a secure password."),
        'bee user-password "Joe Bloggs"' => bt("Give the 'Joe Bloggs' user account a random password."),
      ),
    ),
    'user-login' => array(
      'description' => bt('Display a login link for a given user.'),
      'callback' => 'user_login_bee_callback',
      'arguments' => array(
        'username' => bt('The username of the user account to login as. Leave blank to login as User 1.'),
      ),
      'optional_arguments' => array('username'),
      'aliases' => array('uli'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee user-login bob' => bt("Generate and display a one-time login link for the user account named 'bob'."),
        'bee user-login' => bt("Generate and display a one-time login link for User 1."),
      ),
    ),
  );
}

/**
 * Command callback: List all user accounts.
 */
function users_bee_callback($arguments, $options) {
  $rows = array();
  $users = entity_load_multiple('user');

  foreach ($users as $user) {
    // Skip the 'anonymous' user.
    if ($user->uid == 0) {
      continue;
    }

    // Add row.
    $rows[] = array(
      array('value' => $user->uid),
      array('value' => $user->name),
      array('value' => $user->mail),
      array('value' => ($user->login > 0) ? format_date($user->login, 'short') : bt('Never')),
      array('value' => ($user->status > 0) ? bt('Active') : bt('Blocked')),
    );
  }

  // Get table header.
  $header = array(
    array('value' => bt('User ID')),
    array('value' => bt('Username')),
    array('value' => bt('Email')),
    array('value' => bt('Last Login')),
    array('value' => bt('Status')),
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
 * Command callback: Reset the login password for a given user.
 */
function user_password_bee_callback($arguments, $options) {
  // Load the user account.
  $user = user_load_by_name($arguments['username']);
  if (!$user) {
    bee_message(bt("User account '!user' cound not be found.", array(
      '!user' => $arguments['username'],
    )), 'error');
    return;
  }

  // Get or generate the new password.
  $password = !empty($arguments['password']) ? $arguments['password'] : user_password(20);

  // Save the new password to the user account.
  $user->pass = $password;
  user_save($user);

  bee_message(bt("The password for '!user' has been reset.", array(
    '!user' => $user->name,
  )), 'success');

  // Display the generated password.
  if (empty($arguments['password'])) {
    bee_message(bt('The new password is: !password', array(
      '!password' => $password,
    )));
  }
}

/**
 * Command callback: Display a login link for a given user.
 */
function user_login_bee_callback($arguments, $options) {
  // Include `settings.php`. This is required for the generated login link to
  // work properly.
  global $_bee_backdrop_root, $_bee_backdrop_site, $settings;
  $settings_path = $_bee_backdrop_root;
  $settings_path .= !empty($_bee_backdrop_site) ? '/sites/' . $_bee_backdrop_site : '';
  $settings_path .= '/settings.php';
  include $settings_path;

  // Load the user account.
  if (!empty($arguments['username'])) {
    $user = user_load_by_name($arguments['username']);
  }
  else {
    // Load User 1 if no username provided.
    $user = user_load(1);
  }
  if (!$user) {
    bee_message(bt("User account '!user' cound not be found.", array(
      '!user' => $arguments['username'],
    )), 'error');
    return;
  }

  // Generate and display the login link.
  $url = user_pass_reset_url($user) . '/login';
  return array(
    array(
      'type' => 'text',
      'variables' => array('value' => bt("Use the following link to login as '!user':", array(
        '!user' => $user->name,
      ))),
    ),
    array(
      'type' => 'text',
      'variables' => array('value' => $url),
    ),
  );
}
