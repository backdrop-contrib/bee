<?php
/**
 * @file
 * PHPUnit tests for Backdrop Console Projects commands.
 */

use PHPUnit\Framework\TestCase;

class ProjectsCommandsTest extends TestCase {

  /**
   * Make sure that the projects command works.
   */
  public function test_projects_command_works() {
    // All projects.
    $output_all = shell_exec('b projects');
    $this->assertRegExp('/| admin_bar +| Administration Bar +| module +| Enabled +|/', $output_all);
    $this->assertRegExp('/| bartik +| Bartik +| theme +| Disabled +|/', $output_all);
    $this->assertRegExp('/| moscone +| Moscone +| layout +| Enabled +|/', $output_all);

    // Specific project.
    $output_project = shell_exec('b projects contact');
    $this->assertRegExp('/Name +Contact/', $output_project);
    $this->assertRegExp('/Description +Enables the use of both personal and site-wide contact forms./', $output_project);

    // Just modules.
    $output_modules = shell_exec('b projects --type=module');
    $this->assertRegExp('/| taxonomy +| Taxonomy +| module +| Enabled +|/', $output_modules);
    $this->assertStringNotContainsString(' | theme ', $output_modules);
    $this->assertStringNotContainsString(' | layout ', $output_modules);
  }

  /**
   * Make sure that the enable command works.
   */
  public function test_enable_command_works() {
    $output_single = shell_exec('b enable contact');
    $this->assertStringContainsString("The 'Contact' module was enabled.", $output_single);

    $output_multiple = shell_exec('b enable book bartik');
    $this->assertStringContainsString("The 'Book' module was enabled.", $output_multiple);
    $this->assertStringContainsString("The 'Bartik' theme was enabled.", $output_multiple);
  }

  /**
   * Make sure that the disable command works.
   */
  public function test_disable_command_works() {
    $output_single = shell_exec('b disable contact');
    $this->assertStringContainsString("The 'Contact' module was disabled.", $output_single);

    $output_multiple = shell_exec('b disable book bartik');
    $this->assertStringContainsString("The 'Book' module was disabled.", $output_multiple);
    $this->assertStringContainsString("The 'Bartik' theme was disabled.", $output_multiple);
  }

  /**
   * Make sure that the uninstall command works.
   */
  public function test_uninstall_command_works() {
    $output = shell_exec('b uninstall contact book');
    $this->assertStringContainsString("The 'Contact' module was uninstalled.", $output);
    $this->assertStringContainsString("The 'Book' module was uninstalled.", $output);
  }

  /**
   * Make sure that the theme-default command works.
   */
  public function test_theme_default_command_works() {
    $output = shell_exec('b theme-default bartik');
    $this->assertStringContainsString("'Bartik' was set as the default theme.", $output);

    // Reset theme.
    exec('b theme-default basis');
    exec('b disable bartik');
  }

  /**
   * Make sure that the theme-admin command works.
   */
  public function test_theme_admin_command_works() {
    $output = shell_exec('b theme-admin basis');
    $this->assertStringContainsString("Basis' was set as the admin theme.", $output);

    // Reset theme.
    exec('b theme-admin seven');
  }

}
