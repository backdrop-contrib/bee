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
          'description' => bt('Get the permissions granted to this role.'),
          'value' => bt('Role'),
        ),
      ),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee roles' => bt('Display a list of all roles with the permissions for the current site.'),
        'bee roles --role=editor' => bt('Display all the permissions for the editor role.'),
      ),
    ),
    'permissions' => array(
      'description' => bt('List all permissions of the modules.'),
      'callback' => 'permissions_bee_callback',
      'group' => 'roles',
      'aliases' => array('pls', 'permissions-list'),
      'options' => array(
        'module' => array(
          'description' => bt('Get the permissions for this module.'),
          'value' => bt('Module'),
        ),
      ),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee permissions' => bt('Display a list of all permissions of the modules for the current site.'),
        'bee permissions --module=node' => bt("Display a list of all permissions from the 'node' module for the current site."),
      ),
    ),

    'role-create' => array(
      'description' => bt('Add a role.'),
      'callback' => 'role_add_bee_callback',
      'group' => 'roles',
      'arguments' => array(
        'role' => bt('Role to add.'),
      ),
      'aliases' => array('rcrt'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee role-create manager' => bt("Add role 'manager'."),
      ),
    ),
    'role-delete' => array(
      'description' => bt('Delete a role.'),
      'callback' => 'role_delete_bee_callback',
      'group' => 'roles',
      'arguments' => array(
        'role' => bt('Role to delete.'),
      ),
      'aliases' => array('rdel'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee role-delete manager' => bt("Delete role 'manager'."),
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
        'bee role-add-perm  \'post comments\' \'anonymous\'' => bt('Allow anonymous users to post comments.'),
        'bee role-add-perm  "\'view own unpublished content\' , \'view any unpublished content\' , \'view revisions\'" \'anonymous\'' => bt("Grant multiple permissions to the 'anonymous' role."),
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
        'bee role-remove-perm \'access content\' \'anonymous\'' => bt("Hide content from anonymous users."),
        'bee role-remove-perm  "\'view own unpublished content\' , \'view any unpublished content\' , \'view revisions\'" \'anonymous\'' => bt("Remove multiple permissions from the 'anonymous' role."),
      ),
    ),
  );
}

/**
 * Command callback: List all roles.
 */
function roles_bee_callback($arguments, $options) {
  $rows = array();
  // Get a list of all roles including the anonymous role.
  $roles = user_roles(FALSE);
  if (!empty($options['role'])) {
    // If a role has been specified, check if exists, and return error if
    // not.
    $role = strtolower($options['role']);
    if (!array_key_exists($role, $roles)) {
      bee_message(bt("The role '!role' doesn't exist!", array(
        '!role' => $role,
      )), 'error');
      return;
    }
    $permissions = implode('\',\'', user_role_permissions(array($role)));
    if (!empty($permissions)) {
      $permissions = '\'' . $permissions . '\'';
      $output = array(array(
        'type' => 'text',
        'variables' => array(
          'value' => bt("The '!role' role has the following permissions granted: !permissions", array(
            '!role' => $options['role'],
            '!permissions' => $permissions,
          )),
        ),
      ),);
      return $output;
    }
    else {
      bee_message(bt("The role '!role' doesn't have any permissions.", array(
        '!role' => $options['role'],
      )), 'info');
      return;
    }
    return $output;
  }
  else {
    $output = array();
    foreach ($roles as $role => $role_label) {
      $permissions = implode('\' , \'', user_role_permissions(array($role)));
      if (!empty($permissions)) {
        $permissions = '\''.$permissions.'\'';
      }
      $output[] = array(
        'type' => 'text',
        'variables' => array(
          'value' => strtoupper($role) . ': ' . $permissions  . "\n",
        ),
      );
    }
    return($output);
  }
}

/**
 * Command callback: List all permissions of the modules.
 */
function permissions_bee_callback($arguments, $options) {
  $module_list = module_list();
  if (!empty($options['module'])) {
    // If a module has been specified, check it is active, and return error if
    // not.
    $module = strtolower($options['module']);
    if (!array_key_exists($module, $module_list)) {
      bee_message(bt("The module '!module' is not enabled.", array(
        '!module' => $module,
      )), 'error');
      return;
    }
    // Check if the module has permissions defined.
    $permissions = module_invoke($module, 'permission');
    if (!empty($permissions)) {
      // Convert the permissions array to a comma separated list of quoted
      // permissions.
      $permissions = implode('\',\'', array_keys($permissions));
      $permissions = '\'' . $permissions . '\'';
      // Prepare the command output.
      $output = array(array(
        'type' => 'text',
        'variables' => array(
          'value' => bt("The module '!module' has the following permissions: !permissions ", array(
            '!module' => $module,
            '!permissions' => $permissions,
          )),
        ),
      ),);
      return $output;
    }
    else {
      // If the module has no defined permissions, return an error.
      bee_message(bt("The module '!module' has no permissions defined.", array(
        '!module' => $module,
      )), 'error');
      return;
    }
  }
  else {
    // If no module is defined, show all modules with permissions.
    $output = array();
    // Check all active modules for defined permissions.
    foreach ($module_list as $key => $module) {
      $permissions = module_invoke($module, 'permission');
      if (!empty($permissions)) {
        // If permissions exist for the module, convert the permissions array
        // to a comma separated list of quoted permissions.
        $permissions = implode('\',\'', array_keys($permissions));
        $permissions = '\''.$permissions.'\'';
        // Prepare the command output.
        $output[] = array(
          'type' => 'text',
          'variables' => array(
            'value' => strtoupper($module) . ': ' . $permissions  . "\n",
          ),
        );
      }
    }
    return $output;
  }
}

/**
 * Command callback: Add a role
 */
function role_add_bee_callback($arguments, $options) {
  $roles = user_roles(FALSE);
  if (empty($roles[$arguments['role']])) {
    $role = new stdClass();
    $role->name = $arguments['role'];
    $role->label = $arguments['role'];
    if (user_role_save($role)) {
      bee_message(bt("The '!role' role has been created.", array(
        '!role' => $arguments['role'],
      )), 'success');
    }
    else {
      bee_message(bt("The '!role' role creation failed.", array(
        '!role' => $arguments['role'],
      )), 'error');
    }
  }
  else {
    bee_message(bt("The '!role' role already exists.", array(
      '!role' => $arguments['role'],
    )), 'error');
  }
}

/**
 * Command callback: Delete a role
 */
function role_delete_bee_callback($arguments, $options) {
  $roles = user_roles(FALSE);
  if (empty($roles[$arguments['role']])) {
    bee_message(bt("The '!role' role does not exist.", array(
      '!role' => $arguments['role'],
    )));
    return;
  }

  // Is the role a system role?
  $admin_role = config_get('system.core', 'user_admin_role');
  $system_roles = array(
    'anonymous',
    'authenticated',
    $admin_role,
  );

  if (in_array($arguments['role'], $system_roles)) {
    bee_message(bt("The '!role' role is required by the system and cannot be deleted.", array(
      '!role' => $arguments['role'],
    )), 'error');
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
      bee_message(bt("The '!role' role is in use by user '!user'.", array(
        '!role' => $arguments['role'],
        '!user' => $user->name,
      )), 'error');
      return;
    }
  }
  // Delete the role.
  user_role_delete($arguments['role']);
  $roles = user_roles(FALSE);
  if (empty($roles[$arguments['role']])) {
    bee_message(bt("The '!role' role has been deleted.", array(
      '!role' => $arguments['role'],
    )), 'success');
  }
  else {
    bee_message(bt("The '!role' role could not be deleted.", array(
      '!role' => $arguments['role'],
    )), 'error');
  }
}

/**
 * Command callback: Add permission(s) to a role
 */
function role_add_permission_bee_callback($arguments, $options) {
  $roles = user_roles(FALSE);
  // Does the role exist?
  if (empty($roles[$arguments['role']])) {
    bee_message(bt("The '!role' role does not exist.", array(
      '!role' => $arguments['role'],
    )));
    return;
  }
  // Do the permission exist?
  $module_list = module_list();
  $permissions = array();
  foreach ($module_list as $key => $module) {
    $modify_permission = module_invoke($module, 'permission');
    if (!empty($modify_permission)) {
      $permissions = array_merge($permissions, array_keys($modify_permission));
    }
  }
  $permissions = explode(',', (str_replace('\'', '', (str_replace('"', '', $arguments['permissions'])))));
  foreach ($permissions as $permission) {
    if (! in_array("$permission", $permissions)) {
      bee_message(bt("The '!permission' permission could not be found!", array(
        '!permission' => $permission,
      )), 'error');
      return;
    }
  }
  // Grant the permissions.
  user_role_grant_permissions($arguments['role'], $permissions);
  $role = array($arguments['role']);
  $permissions = implode('\',\'', user_role_permissions($role));
  if (!empty($permissions)) {
    $permissions = '\''.$permissions.'\'';
  }
  bee_message(bt("The '!role' role has the following permissions granted: !permissions", array(
    '!role' => $arguments['role'],
    '!permissions' => $permissions,
  )), 'success');
}

/**
 * Command callback: Remove permission(s) from a role.
 */
function role_remove_permission_bee_callback($arguments, $options) {
  $roles = user_roles(FALSE);
  // Do the role exists?
  if (empty($roles[$arguments['role']])) {
    bee_message(bt("The '!role' role does not exist.", array(
      '!role' => $arguments['role'],
    )));
    return;
  }
  $grant_permissions = array();
  $permissions = explode(',', (str_replace('\'', '', (str_replace('"', '', $arguments['permissions'])))));
  $role = array($arguments['role']);
  $current_permissions = user_role_permissions($role);
  // First set all to TRUE.
  // Any value that evaluates to TRUE will cause the permission to be granted.
  if ( ! empty($current_permissions)) {
    foreach ($permissions as $permission) {
      $grant_permissions[$permission] = TRUE;
    }
  }
  foreach ($permissions as $permission) {
    if (! in_array("$permission", $current_permissions)) {
      bee_message(bt("The '!permission' permission has not be granted!", array(
        '!permission' => $permission,
      )), 'error');
      return;
    }
    else {
      $grant_permissions[$permission] = FALSE;
    }
  }
  user_role_change_permissions($arguments['role'], $grant_permissions);
  $permissions = implode('\',\'', user_role_permissions($role));
  if (!empty($permissions)) {
    $permissions = '\''.$permissions.'\'';
  }
  bee_message(bt("The '!role' role has the following permissions granted: !perms", array(
    '!role' => $arguments['role'],
    '!perms' => $permissions,
  )), 'success');
}
