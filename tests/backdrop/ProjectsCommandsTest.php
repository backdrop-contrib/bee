<?php
/**
 * @file
 * PHPUnit tests for Bee Projects commands.
 */

use PHPUnit\Framework\TestCase;
/**
 * Test Bee's commands for gettings information on installed projects.
 */
class ProjectsCommandsTest extends TestCase {

  /**
   * Make sure that the projects command works.
   */
  public function test_projects_command_works() {
    // All projects.
    $output_all = shell_exec('bee projects');
    $this->assertRegExp('/| admin_bar +| Administration Bar +| module +| Enabled +|/', (string) $output_all);
    $this->assertRegExp('/| bartik +| Bartik +| theme +| Disabled +|/', (string) $output_all);
    $this->assertRegExp('/| moscone +| Moscone +| layout +| Enabled +|/', (string) $output_all);

    // Specific project.
    $output_project = shell_exec('bee projects contact');
    $this->assertRegExp('/Name +Contact/', (string) $output_project);
    $this->assertRegExp('/Description +Enables the use of both personal and site-wide contact forms./', (string) $output_project);

    // Just modules.
    $output_modules = shell_exec('bee projects --type=module');
    $this->assertRegExp('/| taxonomy +| Taxonomy +| module +| Enabled +|/', (string) $output_modules);
    $this->assertStringNotContainsString(' | theme ', (string) $output_modules);
    $this->assertStringNotContainsString(' | layout ', (string) $output_modules);
  }

  /**
   * Make sure that the enable command works.
   */
  public function test_enable_command_works() {
    $output_single = shell_exec('bee enable contact');
    $this->assertStringContainsString("The 'Contact' module was enabled.", (string) $output_single);

    $output_multiple = shell_exec('bee enable book bartik');
    $this->assertStringContainsString("The 'Book' module was enabled.", (string) $output_multiple);
    $this->assertStringContainsString("The 'Bartik' theme was enabled.", (string) $output_multiple);
  }

  /**
   * Make sure that the disable command works.
   */
  public function test_disable_command_works() {
    $output_single = shell_exec('bee disable contact');
    $this->assertStringContainsString("The 'Contact' module was disabled.", (string) $output_single);

    $output_multiple = shell_exec('bee disable book bartik');
    $this->assertStringContainsString("The 'Book' module was disabled.", (string) $output_multiple);
    $this->assertStringContainsString("The 'Bartik' theme was disabled.", (string) $output_multiple);
  }

  /**
   * Make sure that the uninstall command works.
   */
  public function test_uninstall_command_works() {
    $output = shell_exec('bee uninstall contact book');
    $this->assertStringContainsString("The 'Contact' module was uninstalled.", (string) $output);
    $this->assertStringContainsString("The 'Book' module was uninstalled.", (string) $output);
  }

}
