<?php
/**
 * @file
 * Command(s) for working with Backdrop projects.
 */

/**
 * Implements hook_bee_command().
 */
function projects_bee_command() {
  return array(
    'projects' => array(
      'description' => bt('Display information about available projects (modules, themes, layouts).'),
      'callback' => 'projects_bee_callback',
      'group' => 'projects',
      'arguments' => array(
        'project' => bt('The name of a project to display information for. Leave blank to list information about all projects.'),
      ),
      'optional_arguments' => array('project'),
      'options' => array(
        'type' => array(
          'description' => bt("Restrict list to projects of a given type: 'module', 'theme' or 'layout'."),
          'value' => bt('Type'),
        ),
      ),
      'aliases' => array('pml', 'pmi', 'project', 'pm-list', 'pm-info'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee projects webform' => bt('Show information about the Webform module.'),
        'bee projects --type=layout' => bt('Show information about all available layouts.'),
        'bee projects' => bt('Show information about all available projects.'),
      ),
    ),
    'enable' => array(
      'description' => bt('Enable one or more projects (modules, themes, layouts).'),
      'callback' => 'enable_bee_callback',
      'group' => 'projects',
      'arguments' => array(
        'projects' => bt('One or more projects to enable.'),
      ),
      'multiple_argument' => 'projects',
      'options' => array(
        'no-dependency-checking' => array(
          'description' => bt('Disable dependency-checking and enable module(s) regardless. This could cause problems if there are missing dependencies. Use with caution.'),
          'short' >= 'n',
        ),
      ),
      'aliases' => array('en', 'pm-enable'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee enable webform' => bt('Enable the Webform module.'),
        'bee enable --no-dependency-checking rules' => bt('Enable the Rules module, regardless of whether or not its dependencies are available.'),
        'bee enable simplify thesis bamboo' => bt('Enable the Simplify module, Thesis theme, and Bamboo layout.'),
      ),
    ),
    'disable' => array(
      'description' => bt('Disable one or more projects (modules, themes, layouts).'),
      'callback' => 'disable_bee_callback',
      'group' => 'projects',
      'arguments' => array(
        'projects' => bt('One or more projects to disable.'),
      ),
      'multiple_argument' => 'projects',
      'options' => array(
        'no-dependency-checking' => array(
          'description' => bt('Disable dependency-checking and disable module(s) regardless. This could cause problems if there are other enabled modules that depend on this one. Use with caution.'),
          'short' >= 'n',
        ),
      ),
      'aliases' => array('dis', 'pm-disable'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee disable webform' => bt('Disable the Webform module.'),
        'bee disable --no-dependency-checking rules' => bt('Disable the Rules module, regardless of whether or not other modules depend on it.'),
        'bee disable simplify thesis bamboo' => bt('Disable the Simplify module, Thesis theme, and Bamboo layout.'),
      ),
    ),
    'uninstall' => array(
      'description' => bt('Uninstall one or more modules.'),
      'callback' => 'uninstall_bee_callback',
      'group' => 'projects',
      'arguments' => array(
        'modules' => bt('One or more modules to uninstall.'),
      ),
      'multiple_argument' => 'modules',
      'options' => array(
        'no-dependency-checking' => array(
          'description' => bt('Disable dependency-checking and uninstall module(s) regardless. This could cause problems if there are other installed modules that depend on this one. Use with caution.'),
          'short' >= 'n',
        ),
      ),
      'aliases' => array('pmu', 'pm-uninstall'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee uninstall webform' => bt('Uninstall the Webform module.'),
        'bee uninstall --no-dependency-checking rules' => bt('Uninstall the Rules module, regardless of whether or not other modules depend on it.'),
        'bee uninstall simplify thesis bamboo' => bt('Uninstall the Simplify module, Thesis theme, and Bamboo layout.'),
      ),
    ),
  );
}

/**
 * Command callback: Display information about available projects (modules,
 * themes, layouts).
 */
function projects_bee_callback($arguments, $options) {
  $rows = array();

  // Show information about a single project.
  if (!empty($arguments['project'])) {
    return projects_bee_show_project($arguments['project']);
  }

  // Get module data.
  if (!isset($options['type']) || $options['type'] == 'module') {
    $modules = system_rebuild_module_data();

    foreach ($modules as $module) {
      if (!empty($module->info['hidden'])) {
        continue;
      }

      $rows[] = array(
        array('value' => $module->name),
        array('value' => $module->info['name']),
        array('value' => $module->type),
        array('value' => $module->status ? bt('Enabled') : bt('Disabled')),
        array('value' => isset($module->info['version']) ? $module->info['version'] : ''),
      );
    }
  }

  // Get theme data.
  if (!isset($options['type']) || $options['type'] == 'theme') {
    $themes = system_rebuild_theme_data();

    foreach ($themes as $theme) {
      if (!empty($theme->info['hidden'])) {
        continue;
      }

      $rows[] = array(
        array('value' => $theme->name),
        array('value' => $theme->info['name']),
        array('value' => $theme->type),
        array('value' => $theme->status ? bt('Enabled') : bt('Disabled')),
        array('value' => isset($theme->info['version']) ? $theme->info['version'] : ''),
      );
    }
  }

  // Get layout data.
  if (!isset($options['type']) || $options['type'] == 'layout') {
    $layouts = layout_get_layout_template_info();
    $disabled_layouts = config_get('layout.settings', 'excluded_templates');

    foreach ($layouts as $layout) {
      if (!empty($layout['hidden'])) {
        continue;
      }

      $rows[] = array(
        array('value' => $layout['name']),
        array('value' => $layout['title']),
        array('value' => $layout['type']),
        array('value' => !in_array($layout['name'], $disabled_layouts) ? bt('Enabled') : bt('Disabled')),
        array('value' => isset($layout['version']) ? $layout['version'] : ''),
      );
    }
  }

  return array(
    array(
      'type' => 'table',
      'variables' => array(
        'rows' => $rows,
        'header' => array(
          array('value' => bt('Project')),
          array('value' => bt('Name')),
          array('value' => bt('Type')),
          array('value' => bt('Status')),
          array('value' => bt('Version')),
        ),
      ),
    ),
  );
}

/**
 * Command callback: Enable one or more projects (modules, themes, layouts).
 */
function enable_bee_callback($arguments, $options) {
  $modules = system_rebuild_module_data();
  $themes = system_rebuild_theme_data();
  $layouts = layout_get_layout_template_info();
  $disabled_layouts = array();
  $enable_dependencies = !isset($options['no-dependency-checking']);
  $flush_caches = FALSE;

  foreach ($arguments['projects'] as $project) {
    $already_enabled = FALSE;
    $success = FALSE;

    // Get project type.
    if (!$type = projects_bee_get_type($project)) {
      bee_message(bt("Project '!name' could not be found.", array(
        '!name' => $project,
      )), 'error');
      continue;
    }

    switch ($type) {
      case 'module':
        // Get module info.
        $module = $modules[$project];
        $name = $module->info['name'];

        // Check if already enabled.
        if ($module->status) {
          $already_enabled = TRUE;
          break;
        }

        // Enable module.
        if (module_enable(array($project), $enable_dependencies)) {
          $success = TRUE;
        }
        else {
          bee_message(bt("The '!name' module could not be enabled due to missing dependencies.", array(
            '!name' => $name,
          )), 'error');
        }

        break;
      case 'theme':
        // Get theme info.
        $theme = $themes[$project];
        $name = $theme->info['name'];

        // Check if already enabled.
        if ($theme->status) {
          $already_enabled = TRUE;
          break;
        }

        // Enable theme.
        theme_enable(array($project));
        $success = TRUE;

        break;
      case 'layout':
        $config = config('layout.settings');
        if (empty($disabled_layouts)) {
          $disabled_layouts = $config->get('excluded_templates');
        }

        // Get layout info.
        $layout = $layouts[$project];
        $name = $layout['title'];

        // Check if already enabled.
        if (!in_array($project, $disabled_layouts)) {
          $already_enabled = TRUE;
          break;
        }

        // Enable layout (i.e. remove it from the disabled list).
        $key = array_search($project, $disabled_layouts);
        unset($disabled_layouts[$key]);
        $config->set('excluded_templates', $disabled_layouts);
        $config->save();
        $success = TRUE;

        break;
    }

    // Display a message if the project was already enabled.
    if ($already_enabled) {
      bee_message(bt("The '!name' !type is already enabled.", array(
        '!name' => $name,
        '!type' => $type,
      )), 'info');
    }

    // Display a message if the project was successfully enabled.
    if ($success) {
      bee_message(bt("The '!name' !type was enabled.", array(
        '!name' => $name,
        '!type' => $type,
      )), 'success');

      $flush_caches = TRUE;
    }
  }

  // Flush caches once after all projects have been enabled.
  if ($flush_caches) {
    backdrop_flush_all_caches();
  }
}

/**
 * Command callback: Disable one or more projects (modules, themes, layouts).
 */
function disable_bee_callback($arguments, $options) {
  $modules = system_rebuild_module_data();
  $themes = system_rebuild_theme_data();
  $layouts = layout_get_layout_template_info();
  $disabled_layouts = array();
  $disable_dependents = !isset($options['no-dependency-checking']);
  $flush_caches = FALSE;

  foreach ($arguments['projects'] as $project) {
    $already_disabled = FALSE;
    $success = FALSE;

    // Get project type.
    if (!$type = projects_bee_get_type($project)) {
      bee_message(bt("Project '!name' could not be found.", array(
        '!name' => $project,
      )), 'error');
      continue;
    }

    switch ($type) {
      case 'module':
        // Get module info.
        $module = $modules[$project];
        $name = $module->info['name'];

        // Check if already disabled.
        if (!$module->status) {
          $already_disabled = TRUE;
          break;
        }

        // Disable module.
        module_disable(array($project), $disable_dependents);
        $success = TRUE;

        break;
      case 'theme':
        // Get theme info.
        $theme = $themes[$project];
        $name = $theme->info['name'];

        // Check if already disabled.
        if (!$theme->status) {
          $already_disabled = TRUE;
          break;
        }

        // Disable theme.
        theme_disable(array($project));
        $success = TRUE;

        break;
      case 'layout':
        $config = config('layout.settings');
        if (empty($disabled_layouts)) {
          $disabled_layouts = $config->get('excluded_templates');
        }

        // Get layout info.
        $layout = $layouts[$project];
        $name = $layout['title'];

        // Check if already disabled.
        if (in_array($project, $disabled_layouts)) {
          $already_disabled = TRUE;
          break;
        }

        // Disable layout (i.e. add it to the disabled list).
        $disabled_layouts[] = $project;
        $config->set('excluded_templates', $disabled_layouts);
        $config->save();
        $success = TRUE;

        break;
    }

    // Display a message if the project was already disabled.
    if ($already_disabled) {
      bee_message(bt("The '!name' !type is already disabled.", array(
        '!name' => $name,
        '!type' => $type,
      )), 'info');
    }

    // Display a message if the project was successfully disabled.
    if ($success) {
      bee_message(bt("The '!name' !type was disabled.", array(
        '!name' => $name,
        '!type' => $type,
      )), 'success');

      $flush_caches = TRUE;
    }
  }

  // Flush caches once after all projects have been disabled.
  if ($flush_caches) {
    backdrop_flush_all_caches();
  }
}

/**
 * Command callback: Uninstall one or more modules.
 */
function uninstall_bee_callback($arguments, $options) {
  $modules = array();
  $uninstall_dependents = !isset($options['no-dependency-checking']);
  $flush_caches = FALSE;

  foreach ($arguments['modules'] as $module_name) {
    // Get module info.
    if (empty($modules)) {
      $modules = system_rebuild_module_data();
    }
    if (!isset($modules[$module_name])) {
      bee_message(bt("The '!name' module could not be found.", array(
        '!name' => $module_name,
      )), 'error');
      continue;
    }
    $module = $modules[$module_name];

    // Make sure module is disabled.
    if ($module->status) {
      bee_message(bt("The '!name' module is still enabled and so cannot be uninstalled. Disable it and try again.", array(
        '!name' => $module->info['name'],
      )), 'error');
      break;
    }

    // Check if already uninstalled.
    if ($module->schema_version == SCHEMA_UNINSTALLED) {
      bee_message(bt("The '!name' module is already uninstalled.", array(
        '!name' => $module->info['name'],
      )), 'info');
      break;
    }

    // Uninstall module.
    if (backdrop_uninstall_modules(array($module_name), $uninstall_dependents)) {
      bee_message(bt("The '!name' module was uninstalled.", array(
        '!name' => $module->info['name'],
      )), 'success');

      $flush_caches = TRUE;
    }
    else {
      bee_message(bt("The '!name' module could not be uninstalled as other enabled modules depend on it.", array(
        '!name' => $module->info['name'],
      )), 'error');
    }
  }

  // Flush caches once after all projects have been disabled.
  if ($flush_caches) {
    backdrop_flush_all_caches();
  }
}

/**
 * Show information about a single project.
 */
function projects_bee_show_project($project) {
  $modules = system_rebuild_module_data();
  $themes = system_rebuild_theme_data();
  $layouts = layout_get_layout_template_info();
  $rows = array();

  // Get project type.
  if (!$type = projects_bee_get_type($project)) {
    bee_message(bt("Project '!name' could not be found.", array(
      '!name' => $project,
    )), 'error');
    return;
  }

  // Get project information.
  switch ($type) {
    case 'module':
      $module = $modules[$project];

      $rows[] = array(
        array('value' => bt('Name')),
        array('value' => $module->info['name']),
      );
      $rows[] = array(
        array('value' => bt('Type')),
        array('value' => ucfirst($type)),
      );
      $rows[] = array(
        array('value' => bt('Project')),
        array('value' => !empty($module->info['project']) ? $module->info['project'] : ''),
      );
      $rows[] = array(
        array('value' => bt('Description')),
        array('value' => !empty($module->info['description']) ? $module->info['description'] : ''),
      );
      $rows[] = array(
        array('value' => bt('Configure')),
        array('value' => !empty($module->info['configure']) ? $module->info['configure'] : ''),
      );
      $rows[] = array(
        array('value' => bt('Package')),
        array('value' => !empty($module->info['package']) ? $module->info['package'] : ''),
      );
      $rows[] = array(
        array('value' => bt('Tags')),
        array('value' => !empty($module->info['tags']) ? implode(', ', $module->info['tags']) : ''),
      );
      $rows[] = array(
        array('value' => bt('Version')),
        array('value' => !empty($module->info['version']) ? $module->info['version'] : ''),
      );
      $rows[] = array(
        array('value' => bt('Path')),
        array('value' => dirname($module->uri)),
      );
      $rows[] = array(
        array('value' => bt('Status')),
        array('value' => $module->status ? bt('Enabled') : bt('Disabled')),
      );
      $rows[] = array(
        array('value' => bt('Requires')),
        array('value' => !empty($module->requires) ? implode(', ', array_keys($module->requires)) : ''),
      );
      $rows[] = array(
        array('value' => bt('Required by')),
        array('value' => !empty($module->required_by) ? implode(', ', array_keys($module->required_by)) : ''),
      );

      break;
    case 'theme':
      $theme = $themes[$project];

      $rows[] = array(
        array('value' => bt('Name')),
        array('value' => $theme->info['name']),
      );
      $rows[] = array(
        array('value' => bt('Type')),
        array('value' => ucfirst($type)),
      );
      $rows[] = array(
        array('value' => bt('Description')),
        array('value' => !empty($theme->info['description']) ? $theme->info['description'] : ''),
      );
      $rows[] = array(
        array('value' => bt('Version')),
        array('value' => !empty($theme->info['version']) ? $theme->info['version'] : ''),
      );
      $rows[] = array(
        array('value' => bt('Path')),
        array('value' => dirname($theme->uri)),
      );
      $rows[] = array(
        array('value' => bt('Status')),
        array('value' => $theme->status ? bt('Enabled') : bt('Disabled')),
      );

      break;
    case 'layout':
      $layout = $layouts[$project];

      $rows[] = array(
        array('value' => bt('Name')),
        array('value' => $layout['title']),
      );
      $rows[] = array(
        array('value' => bt('Type')),
        array('value' => ucfirst($type)),
      );
      $rows[] = array(
        array('value' => bt('Version')),
        array('value' => $layout['version']),
      );
      $rows[] = array(
        array('value' => bt('Path')),
        array('value' => $layout['path']),
      );
      $rows[] = array(
        array('value' => bt('Default region')),
        array('value' => $layout['default region']),
      );

      break;
  }

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

/**
 * Get a project's type (i.e. module, theme or layout).
 */
function projects_bee_get_type($project) {
  // Get the type from the `system` table.
  $type = db_query('SELECT type FROM {system} WHERE name = :name;', array(
    ':name' => $project,
  ))->fetchField();

  // Because only modules and themes are listed in the `system` table, no type
  // found means the project is either a layout, or doesn't exist.
  if (!$type) {
    if (layout_get_layout_template_info($project)) {
      $type = 'layout';
    }
    else {
      // Project doesn't appear to be a module, theme or layout.
      return FALSE;
    }
  }

  return $type;
}
