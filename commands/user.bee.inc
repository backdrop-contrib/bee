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
      'group' => 'users',
      'aliases' => array('uls', 'user-list'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee users' => bt('Display a list of all user accounts for the current site.'),
      ),
    ),
    'user-create' => array(
      'description' => 'Create a user account with the specified name.',
      'callback' => 'user_bee_create_callback',
      'group' => 'users',
      'aliases' => array('ucrt'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'arguments' => array(
        'username' => bt('The name of the account to add'),
        'mail' => bt('Mail address for this account to add'),
        'password' => bt('The password to use for the given user account. Leave blank to generate a random password.'),
    	),
      'optional_arguments' => array('password'),
      'examples' => array(
        'bee user-create joe joe@example.com P@55w0rd' => bt("The 'joe' user account an insecure password (not recommended)."),
        'bee user-create joe joe@example.com ' => bt("The 'joe' user account, password will be generated."),
      ),
    ),
    'user-cancel' => array(
      'description' => bt('Cancel/remove a user.'),
      'callback' => 'user_cancel_bee_callback',
      'group' => 'users',
      'arguments' => array(
        'username' => bt('A user account to cancel/remove.'),
      ),
      'aliases' => array('ucan'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee user-cancel joe' => bt("Cancel/remove the 'joe' user account."),
      ),
    ),
    'user-block' => array(
      'description' => bt('Block a user.'),
      'callback' => 'user_block_bee_callback',
      'group' => 'users',
      'arguments' => array(
        'username' => bt('A user account to block.'),
      ),
      'aliases' => array('ublk'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee user-block joe' => bt("Block the 'joe' user account."),
      ),
    ),
    'user-unblock' => array(
      'description' => bt('Unblock a user.'),
      'callback' => 'user_unblock_bee_callback',
      'group' => 'users',
      'arguments' => array(
        'username' => bt('The username of the user account to unblock.'),
      ),
      'aliases' => array('uublk'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee user-block joe' => bt("UnBlock the 'joe' user account."),
      ),
    ),
    'user-add-role' => array(
      'description' => bt('Add role to user.'),
      'callback' => 'user_add_role_bee_callback',
      'group' => 'users',
      'arguments' => array(
        'username' => bt('Add role to user.'),
        'role' => bt('Role to add'),
      ),
      'aliases' => array('urole'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee user-add-role joe powersuser' => bt("Add role powersuser to account joe."),
      ),
    ),
    'user-remove-role' => array(
      'description' => bt('Remove a role from a user.'),
      'callback' => 'user_remove_role_bee_callback',
      'group' => 'users',
      'arguments' => array(
        'username' => bt('The user to remove the role '),
        'role' => bt('Role to remove'),
      ),
      'aliases' => array('urrole'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee user-remove-role joe powersuser' => bt('Remove role powersuser from account joe.'),
      ),
    ),
    'user-password' => array(
      'description' => bt('Reset the login password for a given user.'),
      'callback' => 'user_password_bee_callback',
      'group' => 'users',
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
      'group' => 'users',
      'arguments' => array(
        'username' => bt('The username of the user account to login as. Leave blank to login as User 1.'),
      ),
      'optional_arguments' => array('username'),
      'aliases' => array('uli'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee user-login bob' => bt("Generate and display a one-time login link for the user account named 'bob'."),
        'bee user-login' => bt("Generate and display a one-time login link for User 1."),
        'bee user-login --base-url=https://www.example.com' => bt("Generate and display a one-time login link for User 1, setting the base-url option."),
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
    $roles = '';
    foreach($user->roles as $role){
      $roles = $roles.$role.', ';
    }
    // Add row.
    $rows[] = array(
      array('value' => $user->uid),
      array('value' => $user->name),
      array('value' => $user->mail),
      array('value' => $roles),
      array('value' => ($user->login > 0) ? format_date($user->login, 'short') : bt('Never')),
      array('value' => ($user->status > 0) ? bt('Active') : bt('Blocked')),
    );
  }

  // Get table header.
  $header = array(
    array('value' => bt('User ID')),
    array('value' => bt('Username')),
    array('value' => bt('Email')),
    array('value' => bt('Roles')),
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
 * Command callback: Create a user.
 */
function user_bee_create_callback($arguments, $options) {
  $user = user_load_by_name($arguments['username']);
  if ($user) {
    bee_message(bt("User account '!user' allready exits.", array(
      '!user' => $arguments['username'],
    )), 'error');
    return;
  }
  if(empty($arguments['mail'])){
    bee_message(bt("Mail argument missing!", array()), 'error');
    return;
  }
  $check = db_select('users', 'u')
    ->fields('u', array('uid'))
    ->condition('mail', $arguments['mail'], '=')
    ->execute()
    ->fetchField();

  if($check != '') {
    bee_message(bt("A user account with mail '!mail' allready exits.", array(
      '!mail' => $arguments['mail'],
    )), 'error');
    return;
  }
  $password = !empty($arguments['password']) ? $arguments['password'] : user_password(20);
  $new_user = array(
    'name' => $arguments['username'],
    'pass' => $password,
    'mail' => $arguments['mail'],
    'status' => 1,
    'access' => '0',
  );
  if ($account = entity_create('user', $new_user)) {
    $account->save();
    bee_message(bt("User '!user' has been created, with password !pass", array(
     '!user' => $arguments['username'], '!pass' => $password,
    )), 'success');
  }else {
    bee_message(bt("Could not create a new user account with the name, !user", array(
      '!user' => $name,
    )), 'error');
  }
}

/**
 * Command callback: Delete/cancel a given user.
 */
function user_cancel_bee_callback($arguments, $options) {
  // Load the user account.
  $user = user_load_by_name($arguments['username']);
  if (!$user) {
    bee_message(bt("User account '!user' cound not be found.", array(
      '!user' => $arguments['username'],
    )), 'error');
    return;
  }

  user_cancel(array(
   'user_cancel_notify' => TRUE,
   'user_cancel_method' => 'user_cancel_delete',
  ), $user->uid, 'user_cancel_delete');

// Invoke the Batch API, disable backdrop_goto(), and execute.
  $batch = &batch_get();
  $batch['progressive'] = FALSE;
  batch_process();

  bee_message(bt("User '!user' has been removed.", array(
    '!user' => $user->name,
  )), 'success');
}

/**
 * Command callback: Block a given user.
 */
function user_block_bee_callback($arguments, $options) {
  // Load the user account.
  $user = user_load_by_name($arguments['username']);
  if (!$user) {
    bee_message(bt("User account '!user' cound not be found.", array(
      '!user' => $arguments['username'],
    )), 'error');
    return;
  }

  $user->status = 0;
  user_save($user);

  bee_message(bt("User '!user' has been blocked.", array(
    '!user' => $user->name,
  )), 'success');
}

/**
 * Command callback: Unblock a given user.
 */
function user_unblock_bee_callback($arguments, $options) {
  // Load the user account.
  $user = user_load_by_name($arguments['username']);
  if (!$user) {
    bee_message(bt("User account '!user' cound not be found.", array(
      '!user' => $arguments['username'],
    )), 'error');
    return;
  }

  $user->status = 1;
  user_save($user);

  bee_message(bt("User '!user' has been unblocked.", array(
    '!user' => $user->name,
  )), 'success');
}

/**
 * Command callback: Add a role to a given user.
 */
function user_add_role_bee_callback($arguments, $options) {
  $user = user_load_by_name($arguments['username']);
  if (!$user) {
    bee_message(bt("User account '!user' cound not be found.", array(
      '!user' => $arguments['username'],
    )), 'error');
    return;
  }
  $role = new stdClass();
  $role->name = $arguments['role'];
  $role->label = $arguments['role'];
  $roles = user_roles(TRUE);
  if(user_role_save($role)) {
    if(!user_has_role($arguments['role'])){
      if (!in_array($arguments['role'], $user->roles)) {
        $user->roles[] = $arguments['role'];
        user_save($user);
        bee_message(bt("The role !role has been assigned to user '!user'.", array(
          '!user' => $user->name, '!role' => $arguments['role']
        )), 'success');
          }else{
            bee_message(bt("The role !role has allready been assigned to user '!user'.", array(
              '!user' => $user->name, '!role' => $arguments['role']
            )));
      }
    }
  }
}
/**
 * Command callback: Remove a role from a given user.
 */
function user_remove_role_bee_callback($arguments, $options) {
  $user = user_load_by_name($arguments['username']);
  if (!$user) {
    bee_message(bt("User account '!user' cound not be found.", array(
      '!user' => $arguments['username'],
    )), 'error');
    return;
  }
  if (in_array($arguments['role'], $user->roles)) {
    if (($key = array_search($arguments['role'], $user->roles)) !== false) {
      unset($user->roles[$key]);
    }
    user_save($user);
      bee_message(bt("The role !role has been removed from  user '!user'.", array(
        '!user' => $user->name, '!role' => $arguments['role']
      )), 'success');
  }else{
    bee_message(bt("The role !role has allready been removed from  user '!user'.", array(
      '!user' => $user->name, '!role' => $arguments['role']
     )));
  }
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
