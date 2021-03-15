<?php
/**
 * @file
 * PHPUnit tests for Backdrop Console Config commands.
 */

use PHPUnit\Framework\TestCase;

class ConfigCommandsTest extends TestCase {

  /**
   * Make sure that the config-get command works.
   */
  public function test_config_get_command_works() {
    $output_all = shell_exec('b config-get system.core');
    $this->assertStringContainsString("'admin_theme' => 'seven',", $output_all);
    $this->assertStringContainsString("'site_frontpage' => 'home',", $output_all);

    $output_theme = shell_exec('b config-get system.core theme_default');
    $this->assertStringContainsString("'basis'", $output_theme);
  }

  /**
   * Make sure that the config-set command works.
   */
  public function test_config_set_command_works() {
    $file = 'system.core';
    $option = 'site_frontpage';
    $value = 'home';
    $new = 'about';

    // Make sure the current homepage is 'home'.
    $output = shell_exec("b config-get $file $option");
    $this->assertStringContainsString($value, $output);

    // Set a new value for the homepage.
    $output = shell_exec("b config-set $file $option $new");
    $this->assertStringContainsString("'$option' was set to '$new'.", $output);

    // Make sure the new homepage is 'about'.
    $output = shell_exec("b config-get $file $option");
    $this->assertStringContainsString($new, $output);

    // Set frontpage back to 'home' for future tests.
    exec("b config-set $file $option $value");
  }

}
