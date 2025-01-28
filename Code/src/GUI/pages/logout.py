import flet as ft

def logout_dialog(page: ft.Page, show_login_page):
    """
    Function that displays a logout confirmation dialog.

    Args:
        page (ft.Page): Current page to display the dialog.
        show_login_page (function): Function to be called if the user confirms logout.
    """
    def handle_logout_confirmation(e):
        # If confirmed, redirect to the login page
        logout_confirmation_dialog.open = False
        show_login_page()
        page.update()

    def handle_logout_cancellation(e):
        # Close the dialog if logout is canceled
        logout_confirmation_dialog.open = False
        page.update()

    # Define the logout confirmation dialog
    logout_confirmation_dialog = ft.AlertDialog(
        modal=True,
        title=ft.Text("Are you sure you want to logout?", size=20, weight="bold"),
        actions=[
            ft.ElevatedButton("Yes", on_click=handle_logout_confirmation),
            ft.TextButton("Cancel", on_click=handle_logout_cancellation)
        ],
        actions_alignment="end"
    )

    # Show the logout confirmation dialog
    page.dialog = logout_confirmation_dialog
    logout_confirmation_dialog.open = True
    page.update()
