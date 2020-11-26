<?php
/**
 * @file
 * Command(s) for working with Backdrop projects.
 */

/**
 * Implements hook_b_command().
 */
function projects_b_command() {
  return array(
    'projects' => array(
      'description' => bt('Display information about available projects (modules, themes, layouts).'),
      'callback' => 'projects_b_callback',
      'arguments' => array(
        'project' => bt('The name of a project to display information for. Leave blank to list information about all projects.'),
      ),
      'optional_arguments' => array('project'),
      'options' => array(
        'type' => array(
          'description' => bt('Restrict list to projects of a given type: `module`, `theme` or `layout`.'),
          'value' => bt('Type'),
        ),
      ),
      'aliases' => array('pm-list', 'pml'),
      'bootstrap' => B_BOOTSTRAP_FULL,
      'examples' => array(
        'b projects webform' => bt('Show information about the Webform module.'),
        'b projects --type=layout' => bt('Show information about all available layouts.'),
        'b projects' => bt('Show information about all available projects.'),
      ),
    ),
    // 'pm-enable' => array(
    //   'bootstrap' => B_BOOTSTRAP_FULL,
    //   'description' => 'Enable one or more modules.',
    //   'arguments' => array(
    //     'extensions' => array(
    //       'description' => 'List of modules.',
    //       'multiple' => TRUE,
    //     ),
    //   ),
    //   'callback' => 'modules_b_enable_callback',
    //   'aliases' => array('en'),
    // ),
    // 'pm-disable' => array(
    //   'bootstrap' => B_BOOTSTRAP_FULL,
    //   'description' => 'Disable one or more modules.',
    //   'arguments' => array(
    //     'extensions' => array(
    //       'description' => 'List of modules.',
    //       'multiple' => TRUE,
    //     ),
    //   ),
    //   'aliases' => array('dis'),
    //   'callback' => 'modules_b_disable_callback',
    // ),
    // 'pm-uninstall' => array(
    //   'bootstrap' => B_BOOTSTRAP_FULL,
    //   'description' => 'Uninstall one or more modules.',
    //   'arguments' => array(
    //     'modules' => array(
    //       'description' => 'List of modules.',
    //       'multiple' => TRUE,
    //     ),
    //   ),
    //   'aliases' => array('pmu'),
    //   'callback' => 'modules_b_uninstall_callback',
    // ),
    // 'pm-info' => array(
    //   'bootstrap' => B_BOOTSTRAP_FULL,
    //   'description' => 'Show detailed info for one or more extensions (modules or themes).',
    //   'arguments' => array(
    //     'modules' => array(
    //       'description' => 'List of modules.',
    //       'multiple' => TRUE,
    //     ),
    //   ),
    //   'aliases' => array('pmi'),
    //   'callback' => 'modules_b_info_callback',
    // ),
  );
}

/**
 * Command callback: Display information about available projects (modules,
 * themes, layouts).
 */
function projects_b_callback($arguments, $options) {
  $rows = array();
  $modules = !isset($options['type']) ? TRUE : $options['type'] == 'module';
  $themes = !isset($options['type']) ? TRUE : $options['type'] == 'theme';
  $layouts = !isset($options['type']) ? TRUE : $options['type'] == 'layout';

  // Get module data.
  if ($modules) {
    $module_data = system_rebuild_module_data();

    foreach ($module_data as $module) {
      if (!empty($module->info['hidden'])) {
        continue;
      }

      $rows[] = array(
        array('value' => $module->name),
        array('value' => $module->info['name']),
        array('value' => $module->type),
        array('value' => $module->status ? bt('Enabled') : bt('Disabled')),
        array('value' => $module->info['version']),
      );
    }
  }

  // Get theme data.
  if ($themes) {
    $theme_data = system_rebuild_theme_data();

    foreach ($theme_data as $theme) {
      if (!empty($theme->info['hidden'])) {
        continue;
      }

      $rows[] = array(
        array('value' => $theme->name),
        array('value' => $theme->info['name']),
        array('value' => $theme->type),
        array('value' => $theme->status ? bt('Enabled') : bt('Disabled')),
        array('value' => $theme->info['version']),
      );
    }
  }

  // Get layout data.
  if ($layouts) {
    $layout_data = layout_get_layout_template_info(NULL, TRUE);
    $disabled_layouts = config_get('layout.settings', 'excluded_templates');

    foreach ($layout_data as $layout) {
      if (!empty($layout['hidden'])) {
        continue;
      }

      $rows[] = array(
        array('value' => $layout['name']),
        array('value' => $layout['title']),
        array('value' => $layout['type']),
        array('value' => !in_array($layout['name'], $disabled_layouts) ? bt('Enabled') : bt('Disabled')),
        array('value' => $layout['version']),
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
 * Callback for pm-enable. Enable module.
 *
 * @param array $arguments
 *   Arguments fro pm-enable command.
 * @param array $options
 *   Options for pm-enable command.
 */
function modules_b_enable_callback($arguments, $options) {
  if (!isset($arguments['extensions'])) {
    b_message('Please provide module name(s)', 'error');
    return;
  }

  $clean_cache = FALSE;
  foreach ($arguments['extensions'] as $project) {
    if (_enable_project($project)) {
      $clean_cache = TRUE;
    }
  }

  if ($clean_cache) {
    backdrop_flush_all_caches();
  }
}

/**
 * Internal function to enable module or theme.
 *
 * @param string $project
 *   The project machine name to be enabled.
 *
 * @return bool
 *   True if enabled otherwise False.
 */
function _enable_project($project) {
  // Rescan for new modules.
  system_rebuild_module_data();

  $query = db_select('system', 's')
      ->fields('s');
  $query->condition('name', $project);
  $query->condition('type', 'module');
  $module = $query->execute()->fetchAssoc();
  if (!$module) {
    return FALSE;
  }

  if ($module['status']) {
    b_message("Failed to enable module " . $module['name'] . ": it is enabled already", 'error');
    return FALSE;
  }

  $rows = array();

  $info = unserialize($module['info']);

  $rows[] = array(
    array('data' => bt('Name'), '#color' => 'bold_blue'),
    $info['name'],
  );
  $rows[] = array(
    array('data' => bt('Description'), '#color' => 'bold_blue'),
    $info['description'],
  );
  $rows[] = array(
    array('data' => bt('Package'), '#color' => 'bold_blue'),
    $info['package'],
  );
  $rows[] = array(
    array('data' => bt('Version'), '#color' => 'bold_blue'),
    $info['version'],
  );

  $output[] = array(
    '#type' => 'table',
    'rows' => $rows,
    'delimiter' => ' : ',
    'delimiter_left' => ' ',
    'delimiter_right' => ' ',
  );

  b_render($output);

  if (!b_confirm(bt('Do you want to enable?'))) {
    b_message(bt('Cancelled to enable !name', array('!name' => $module['name'])));
    return;
  }

  if (module_enable(array($module['name']), FALSE)) {
    b_message("Module " . $module['name'] . " enabled", 'success');
    return TRUE;
  }
  b_message("Failed to enable module " . $module['name'], 'error');
  return FALSE;
}

/**
 * Callback for pm-disable. Disable module or theme.
 *
 * @param array $arguments
 *   Arguments for pm-disable command.
 * @param array $options
 *   Options for pm-disable command.
 */
function modules_b_disable_callback($arguments, $options) {

  if (!isset($arguments['extensions'])) {
    b_message('Please provide project name(s)', 'error');
    return;
  }

  $clean_cache = FALSE;
  foreach ($arguments['extensions'] as $project) {
    if (_disable_project($project)) {
      $clean_cache = TRUE;
    }
  }

  if ($clean_cache) {
    backdrop_flush_all_caches();
  }
}

/**
 * Internal function to disable module or theme.
 *
 * @param sring $project
 *   Project machine name to be disabled.
 */
function _disable_project($project) {
  $query = db_select('system', 's')
      ->fields('s');
  $query->condition('name', $project);
  $query->condition('type', 'module');
  $module = $query->execute()->fetchAssoc();
  if (!$module) {
    return FALSE;
  }

  if (!$module['status']) {
    b_message("Failed to disable module " . $module['name'] . ": it is disabled already", 'error');
    return FALSE;
  }

  $rows = array();

  $info = unserialize($module['info']);

  $rows[] = array(
    array('data' => bt('Name'), '#color' => 'bold_blue'),
    $info['name'],
  );
  $rows[] = array(
    array('data' => bt('Description'), '#color' => 'bold_blue'),
    $info['description'],
  );
  $rows[] = array(
    array('data' => bt('Package'), '#color' => 'bold_blue'),
    $info['package'],
  );
  $rows[] = array(
    array('data' => bt('Version'), '#color' => 'bold_blue'),
    $info['version'],
  );

  $output[] = array(
    '#type' => 'table',
    'rows' => $rows,
    'delimiter' => ' : ',
    'delimiter_left' => ' ',
    'delimiter_right' => ' ',
  );

  b_render($output);

  if (!b_confirm(bt('Do you want to disable?'))) {
    b_message(bt('Cancelled to disable !name', array('!name' => $module['name'])));
    return;
  }

  module_disable(array($module['name']), FALSE);
  b_message("Module " . $module['name'] . " disabled", 'success');
  return TRUE;
}

/**
 * Callback pm-unstall. Uninstall module.
 *
 * @param array $arguments
 *   Arguments for pm-uminstall.
 * @param array $options
 *   Options for pm-uninstall.
 */
function modules_b_uninstall_callback($arguments, $options) {
  if (!isset($arguments['modules'])) {
    b_message('Please provide project name(s)', 'error');
    return;
  }

  $clean_cache = FALSE;
  foreach ($arguments['modules'] as $project) {
    if (_uninstall_project($project)) {
      $clean_cache = TRUE;
    }
  }

  if ($clean_cache) {
    backdrop_flush_all_caches();
  }
}

/**
 * Internal function to disable module or theme.
 *
 * @param string $project
 *   The machine name of the project to be uninstalled.
 */
function _uninstall_project($project) {
  $query = db_select('system', 's')
      ->fields('s');
  $query->condition('name', $project);
  $query->condition('type', 'module');

  $module = $query->execute()->fetchAssoc();
  if (!$module) {
    b_message(bt("Module !name not found", array('!name' => $project)), 'error');
    return FALSE;
  }

  if ($module['status']) {
    b_message("Failed to uninstall module " . $module['name'] . ": it is need to be disabled first", 'error');
    return FALSE;
  }

  $rows = array();

  $info = unserialize($module['info']);

  $rows[] = array(
    array('data' => bt('Name'), '#color' => 'bold_blue'),
    $info['name'],
  );
  $rows[] = array(
    array('data' => bt('Description'), '#color' => 'bold_blue'),
    $info['description'],
  );
  $rows[] = array(
    array('data' => bt('Package'), '#color' => 'bold_blue'),
    $info['package'],
  );
  $rows[] = array(
    array('data' => bt('Version'), '#color' => 'bold_blue'),
    $info['version'],
  );

  $output[] = array(
    '#type' => 'table',
    'rows' => $rows,
    'delimiter' => ' : ',
    'delimiter_left' => ' ',
    'delimiter_right' => ' ',
  );

  b_render($output);

  if (!b_confirm(bt('Do you want to uninstall?'))) {
    b_message(bt('Cancelled to enable !name', array('!name' => $module['name'])));
    return;
  }

  include_once BACKDROP_ROOT . '/core/includes/install.inc';

  if (backdrop_uninstall_modules(array($module['name']), FALSE)) {
    b_message("Module " . $module['name'] . " uninstalled", 'success');
    return TRUE;
  }
  else {
    b_message("Failed to uninstall module " . $module['name'], 'error');
  }
}

/**
 * Callback pm-info. Provide info about extention.
 *
 * @param array $arguments
 *   Arguments for pm-info command.
 * @param array $options
 *   Options for pm-info command.
 */
function modules_b_info_callback($arguments, $options) {
  if (!isset($arguments['modules'])) {
    b_message('Please provide project name(s)', 'error');
    return;
  }
  foreach ($arguments['modules'] as $project) {
    _info_project($project);
  }
}

/**
 * Internal function to provide detailed information about module or theme.
 *
 * @param string $project
 *   The machine name of the project about which info is required.
 */
function _info_project($project) {
  $query = db_select('system', 's')
      ->fields('s');
  $query->condition('name', $project);
  $query->condition('type', 'module');

  $module = $query->execute()->fetchAssoc();
  if (!$module) {
    b_message(bt("Module !name not found", array('!name' => $project)), 'error');
  }
  else {
    $rows = array();

    $info = unserialize($module['info']);

    $rows[] = array(
      array('data' => bt('Name'), '#color' => 'bold_blue'),
      $info['name'],
    );
    $rows[] = array(
      array('data' => bt('Type'), '#color' => 'bold_blue'),
      $info['type'],
    );
    if (isset($info['project'])) {
      $rows[] = array(
        array('data' => bt('Project'), '#color' => 'bold_blue'),
        $info['project'],
      );
    }
    $rows[] = array(
      array('data' => bt('Description'), '#color' => 'bold_blue'),
      $info['description'],
    );
    $rows[] = array(
      array('data' => bt('Package'), '#color' => 'bold_blue'),
      $info['package'],
    );
    $rows[] = array(
      array('data' => bt('Version'), '#color' => 'bold_blue'),
      $info['version'],
    );
    if (isset($info['timestamp'])) {
      $rows[] = array(
        array('data' => bt('Date'), '#color' => 'bold_blue'),
        format_date($info['timestamp'], 'custom', 'Y-m-d'),
      );
    }
    $rows[] = array(
      array('data' => bt('PHP'), '#color' => 'bold_blue'),
      $info['php'],
    );
    $rows[] = array(
      array('data' => bt('Status'), '#color' => 'bold_blue'),
      $module['status'] == '1' ? 'enabled' : 'disabled',
    );
    $rows[] = array(
      array('data' => bt('Path'), '#color' => 'bold_blue'),
      substr($module['filename'], 0, strrpos($module['filename'], '/')),
    );
    $rows[] = array(
      array('data' => bt('Files'), '#color' => 'bold_blue'),
      !empty($info['files']) ? implode(', ', $info['files']) : 'none',
    );
    $rows[] = array(
      array('data' => bt('Requires'), '#color' => 'bold_blue'),
      !empty($info['dependencies']) ? implode(', ', $info['dependencies']) : 'none',
    );
    $rows[] = array(
      array('data' => bt('Configure'), '#color' => 'bold_blue'),
      isset($info['configure']) ? $info['configure'] : 'none',
    );

    $output[] = array(
      '#type' => 'table',
      'rows' => $rows,
      'delimiter' => ' : ',
      'delimiter_left' => ' ',
      'delimiter_right' => ' ',
    );
    b_render($output);
  }
}
