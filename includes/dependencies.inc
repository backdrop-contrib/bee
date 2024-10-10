<?php
/**
 * @file
 * Dependencies on Backdrop Core. These will be removed when Backdrop Core
 * moves these features into hooks available for the contrib space.
 */

/**
 * Builds the full list of dependent modules based on input.
 * This function is copied from Backdrop Core module_disable()
 * until its functionality can be broken out for contrib access.
 *
 * @param array $module_data
 *   A list of all installed modules.
 * @param array $module_list
 *   A list of modules for which to check dependencies.
 *
 * @return array $module_list
 *   A list of modules and their dependent modules.
 */
function bee_get_dependent_modules(array $module_data, array $module_list) {
  // Create an associative array with weights as values.
  $module_list = array_flip(array_values($module_list));

  $profile = backdrop_get_profile();
  // The array is iterated over manually (instead of using a foreach) because
  // modules may be added to the list within the loop and we need to process
  // them.
  while ($module = key($module_list)) {
    next($module_list);
    if (!isset($module_data[$module]) || !$module_data[$module]->status) {
      // This module doesn't exist or is already disabled, skip it.
      unset($module_list[$module]);
      continue;
    }
    $module_list[$module] = $module_data[$module]->sort;

    // Add dependent modules to the list, with a placeholder weight.
    // The new modules will be processed as the while loop continues.
    foreach ($module_data[$module]->required_by as $dependent => $dependent_data) {
      if (!isset($module_list[$dependent]) && $dependent != $profile) {
        $module_list[$dependent] = 0;
      }
    }
  }
  // Sort the module list by pre-calculated weights.
  asort($module_list);
  $module_list = array_keys($module_list);

  return $module_list;
}

/**
 * Builds the full list of module requirements based on input.
 * This function is copied from Backdrop Core module_enable()
 * until its functionality can be broken out for contrib access.
 *
 * @param array $module_data
 *   A list of all installed modules.
 * @param array $module_list
 *   A list of modules for which to check requirements.
 *
 * @return array $module_list
 *   A list of modules and their required modules.
 */
function bee_get_required_modules(array $module_data, array $module_list) {
  // Create an associative array with weights as values.
  $module_list = array_flip(array_values($module_list));

  // The array is iterated over manually (instead of using a foreach) because
  // modules may be added to the list within the loop and we need to process
  // them.
  while ($module = key($module_list)) {
    next($module_list);
    if (!isset($module_data[$module])) {
      // This module is not found in the filesystem, abort.
      return FALSE;
    }
    if ($module_data[$module]->status) {
      // Skip already enabled modules.
      unset($module_list[$module]);
      continue;
    }
    $module_list[$module] = $module_data[$module]->sort;

    // Add dependencies to the list, with a placeholder weight.
    // The new modules will be processed as the while loop continues.
    foreach (array_keys($module_data[$module]->requires) as $dependency) {
      if (!isset($module_list[$dependency])) {
        $module_list[$dependency] = 0;
      }
    }
  }

  // Sort the module list by pre-calculated weights.
  arsort($module_list);
  $module_list = array_keys($module_list);

  return $module_list;
}
