<?php
/**
 * @file
 * Hooks provided by bee, the Backdrop CLI.
 *
 * Implement bee hooks in a `*.bee.inc` file — the same files that provide
 * hook_bee_command() — not in a module's `.module`. bee discovers hook
 * implementations from command files (bee's own `commands/` plus enabled
 * modules'), so bee itself can implement its own hooks.
 */

/**
 * Respond to a project being updated by `bee update`.
 *
 * Invoked after the new files are copied into place, with the site fully
 * bootstrapped, so implementations can adjust the freshly-downloaded code. The
 * canonical use is re-applying local patches that the new release overwrote;
 * bee ships a default implementation (download_bee_post_download()) that does
 * exactly this, from the directory in $info['patches_dir'].
 *
 * @param string $project
 *   The machine name of the project that was updated.
 * @param string $location
 *   The absolute path the project was installed to.
 * @param array $info
 *   The resolved project info (release, branch, download_url, type, ...), plus
 *   'patches_dir': the base directory bee re-applies patches from.
 */
function hook_bee_post_download($project, $location, array $info) {
  // Re-apply a patch this command file carries for the updated project.
  $patch = __DIR__ . '/patches/' . $project . '.patch';
  if (file_exists($patch)) {
    shell_exec('patch -p1 -d ' . escapeshellarg($location) . ' < ' . escapeshellarg($patch));
  }
}
