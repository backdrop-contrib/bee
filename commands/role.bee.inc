<?php
/**
 * @file
 * Command(s) for working with roles.
 */

/**
 * Implements hook_bee_command().
 */
function role_bee_command() {
  return array(
    'roles' => array(
      'description' => bt('List all roles with the permissions.'),
      'callback' => 'roles_bee_callback',
      'group' => 'roles',
      'aliases' => array('rls', 'roles-list'),
      'options' => array(
        'role' => array(
          'description' => bt("Get the permissions granted to this role."),
          'value' => bt('Role'),
        ),
      ),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee roles' => bt('Display a list of all roles with the permissions for the current site.'),
      ),
    ),
    'permissions' => array(
      'description' => bt('List all permissons of the modules.'),
      'callback' => 'permissions_bee_callback',
      'group' => 'roles',
      'aliases' => array('pls', 'permissions-list'),
      'options' => array(
        'module' => array(
          'description' => bt("Get the permissions for this module."),
          'value' => bt('Module'),
        ),
      ),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee permissions' => bt('Display a list of all permissions of the modules for the current site.'),
        'bee permissions --module=node' => bt('Display a list of all permissions  from the module node for the current site.'),
      ),
    ),

    'role-create' => array(
      'description' => bt('Add a role.'),
      'callback' => 'role_add_bee_callback',
      'group' => 'roles',
      'arguments' => array(
        'role' => bt('Role to add'),
      ),
      'aliases' => array('rcrt'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee role-create editor' => bt("Add role 'editor'."),
      ),
    ),
    'role-delete' => array(
      'description' => bt('Delete a role.'),
      'callback' => 'role_delete_bee_callback',
      'group' => 'roles',
      'arguments' => array(
        'role' => bt('Role to delete'),
      ),
      'aliases' => array('rdel'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee role-delete editor' => bt("Delete role 'editor'."),
      ),
    ),
    'role-add-perm' => array(
      'description' => bt('Grant specified permission(s) to a role.'),
      'callback' => 'role_add_permission_bee_callback',
      'group' => 'roles',
      'arguments' => array(
        'permissions' => bt('Permissions'),
        'role' => bt('Role'),
      ),
      'aliases' => array('rap'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee role-add-perm  \'post comments\' \'anonymous user\'' => bt("Allow anon users to post comments."),
        'bee role-add-perm  "\'view own unpublished content\' , \'view any unpublished content\' , \'view revisions\'" \'anonymous user\'' => bt("Grant multiple permissions to the anon users"),
      ),
    ),
    'role-remove-perm' => array(
      'description' => bt('Remove specified permission(s) from a role.'),
      'callback' => 'role_remove_permission_bee_callback',
      'group' => 'roles',
      'arguments' => array(
        'permissions' => bt('Permissions'),
        'role' => bt('Role'),
      ),
      'aliases' => array('rrp'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee role-remove-perm \'access content\' \'anonymous user\'' => bt("Hide content from anon users."),
        'bee role-remove-perm  "\'view own unpublished content\' , \'view any unpublished content\' , \'view revisions\'" \'anonymous user\'' => bt("Remove multiple permissions from the anon users"),
      ),
    ),
  );
}

/**
 * Command callback: List all roles.
 */
function roles_bee_callback($arguments, $options) {
  $rows = array();
  $roles = user_roles(TRUE);
  unset($roles['authenticated']);
  if (!empty($options['role'])) {
    $role = array($options['role']);
    $perms = implode('\',\'', user_role_permissions($role));
    if (!empty($perms)) {
      $perms = '\''.$perms.'\'';
    }
    bee_message(bt("The !role role has the following permissions granted: !perms", array(
      '!role' => $options['role'],
      '!perms' => $perms,
    )));
  }
  else {
    foreach ($roles as $key => $item) {
      if (!empty($key)) {
        $str .= $key.',';
        $keyarray = array($key);
        $perms = implode('\' , \'', user_role_permissions($keyarray));
        if (!empty($perms)) {
          $perms = '\''.$perms.'\'';
        }
        $rows[] = array(
         array('value' => $key),
         array('value' => $perms),
        );
      }
    }
    $header = array(
      array('value' => bt('Role')),
      array('value' => bt('Permissions')),
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
}

/**
 * Command callback: List all permissions of the modules.
 */
function permissions_bee_callback($arguments, $options) {
  $rows = array();
  $module_list = module_list();
  if (!empty($options['module'])) {
    $perms = module_invoke($options['module'], 'permission');
    if (!empty($perms)) {
      $perms = implode('\',\'', array_keys($perms));
      if (!empty($perms)) {
        $perms = '\''.$perms.'\'';
      }
    }
    bee_message(bt("The module !mod has the following permissions: !perms ", array(
      '!mod' => $options['module'],
      '!perms' => $perms,
    )));
    return;
  }
  else {
    foreach ($module_list as $key => $module) {
      $perms = module_invoke($module, 'permission');
      if (!empty($perms)) {
        $perms = implode('\',\'', array_keys($perms));
        if (!empty($perms)) {
          $perms = '\''.$perms.'\'';
        }
      }
      $rows[] = array(
        array('value' => $module),
        array('value' => $perms),
      );
    }
    $header = array(
      array('value' => bt('Module')),
      array('value' => bt('Permissions')),
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
}

/**
 * Command callback: Add a role
 */
function role_add_bee_callback($arguments, $options) {
  $roles = user_roles(TRUE);
  if (empty($roles[$arguments['role']])) {
    $role = new stdClass();
    $role->name = $arguments['role'];
    $role->label = $arguments['role'];
    if (user_role_save($role)) {
      bee_message(bt("The !role role has been created.", array(
        '!role' => $arguments['role'],
      )), 'success');
    }
    else {
      bee_message(bt("The !role role creation failed.", array(
        '!role' => $arguments['role'],
      )), 'error');
    }
  }
  else {
    bee_message(bt("The !role role allready exists.", array(
      '!role' => $arguments['role'],
    )), 'error');
  }
}

/**
 * Command callback: Delete a role
 */
function role_delete_bee_callback($arguments, $options) {
  $roles = user_roles(TRUE);
  if (empty($roles[$arguments['role']])) {
    bee_message(bt("The !role role does not exits.", array(
      '!role' => $arguments['role'],
    )));
    return;
  }
  // Is the role in use by an user?
  $users = entity_load_multiple('user');

  foreach ($users as $user) {
    // Skip the 'anonymous' user.
    if ($user->uid == 0) {
      continue;
    }
    if (in_array($arguments['role'], $user->roles)) {
      bee_message(bt("The !role role is in use by user !user", array(
        '!role' => $arguments['role'],
        '!user' => $user->name,
      )), 'error');
      return;
    }
  }
  // Delete the role.
  user_role_delete($arguments['role']);
  $roles = user_roles(TRUE);
  if (empty($roles[$arguments['role']])) {
    bee_message(bt("The !role role has been deleted.", array(
      '!role' => $arguments['role'],
    )), 'success');
  }
  else {
    bee_message(bt("The !role role could not be deleted", array(
      '!role' => $arguments['role'],
    )), 'error');
  }
}

/**
 * Command callback: Add permission(s) to a role
 */
function role_add_permission_bee_callback($arguments, $options) {
  $roles = user_roles(TRUE);
  if (empty($roles[$arguments['role']])) {
    bee_message(bt("The !role role  does not exits.", array(
      '!role' => $arguments['role'],
    )));
    return;
  }
  // Do the permission exists?
  $module_list = module_list();
  $perms = array();
  foreach ($module_list as $key => $module) {
    $modperms = module_invoke($module, 'permission');
    if (!empty($modperms)) {
      $perms = array_merge($perms, array_keys($modperms));
    }
  }
  $permissions = explode(',', (str_replace('\'', '', (str_replace('"', '', $arguments['permissions'])))));
  foreach ($permissions as $perm) {
    if (! in_array("$perm", $perms)) {
      bee_message(bt("The !perm permission could not be found!", array(
        '!perm' => $perm,
      )), 'error');
      return;
    }
  }
  // Grant the permissions.
  user_role_grant_permissions($arguments['role'], $permissions);
  $role = array($arguments['role']);
  $perms = implode('\',\'', user_role_permissions($role));
  if (!empty($perms)) {
    $perms = '\''.$perms.'\'';
  }
  bee_message(bt("The !role role has the following permissions granted: !perms", array(
    '!role' => $arguments['role'],
    '!perms' => $perms,
  )), 'success');
}

/**
 * Command callback: Remove permission(s) from a role
 */
function role_remove_permission_bee_callback($arguments, $options) {
  $roles = user_roles(TRUE);
  if (empty($roles[$arguments['role']])) {
    bee_message(bt("The !role role  does not exits.", array(
      '!role' => $arguments['role'],
    )));
    return;
  }
  $grantperms = array();
  $permissions = explode(',', (str_replace('\'', '', (str_replace('"', '', $arguments['permissions'])))));
  $role = array($arguments['role']);
  $currentperms = user_role_permissions($role);
  // First set all to TRUE.
  // Any value that evaluates to TRUE will cause the permission to be granted.
  if ( ! empty($currentperms)) {
    foreach ($permissions as $perm) {
      $grantperms[$perm] = TRUE;
    }
  }
  foreach ($permissions as $perm) {
    if (! in_array("$perm", $currentperms)) {
      bee_message(bt("The !perm permission has not be granted!", array(
        '!perm' => $perm,
      )), 'error');
      return;
    }
    else {
      $grantperms[$perm] = FALSE;
    }
  }
  user_role_change_permissions($arguments['role'], $grantperms);
  $perms = implode('\',\'', user_role_permissions($role));
  if (!empty($perms)) {
    $perms = '\''.$perms.'\'';
  }
  bee_message(bt("The !role role has the following permissions granted: !perms", array(
    '!role' => $arguments['role'],
    '!perms' => $perms,
  )), 'success');
}
