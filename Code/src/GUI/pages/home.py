import flet as ft

def home_page():
    return ft.Container(
        content=ft.Column(
            controls=[
                ft.Text("Welcome to Home!", size=30, weight=ft.FontWeight.BOLD),
                ft.Text("This is the main page of the application."),
            ],
            alignment=ft.MainAxisAlignment.START,
        ),
        padding=20,
    )
