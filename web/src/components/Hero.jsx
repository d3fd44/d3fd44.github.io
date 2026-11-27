import profileImage from "../assets/me.png";
import "./Hero.css"

function Hero() {
    return (
        <section className="hero">
            <div className="hero-image">
                <img src={profileImage} alt="My portrait" />
            </div>
            <div className="hero-content">
                <text className="hero-name">Mohammad Momen Defdaa</text>
                <p className="hero-main-subtitle">Systems Engineer | Security Enthusiast | Developer</p>
                <p className="hero-subtitle">i use vim btw</p>
            </div>
        </section>
    );
}

export default Hero;
